// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <kernel/sys/stdint.h>
#include <kernel/sys/globals.h>
#include <kernel/process/manager.h>
#include <kernel/i386/primitives.h>
#include <kernel/libc/enableif.h>
#include <kernel/libc/memory.h>
#include <kernel/log/log.h>
#include <kernel/panic/panic.h>
#include <kernel/libc/slist.h>
#include <kernel/process/process.h>
#include <kernel/mm/virt.h>
#include <kernel/process/fileloader.h>
#include <kernel/process/clonestart.h>
#include <kernel/process/reaper.h>
#include <kernel/boot/phase.h>
#include <kernel/libc/vec.h>
#include <kernel/tasks/scheduler.h>
#include <kernel/tasks/awaker.h>
#include <kernel/tasks/collector.h>
#include <kernel/tasks/deleter.h>
#include <kernel/tasks/keybqueue.h>
#include <kernel/time/manager.h>

LOG_TAG(TIMING, 2);
LOG_TAG(FILEOPS, 0);

// this is THE current process information
process_t *gCurrentProcess;

extern "C" process_t *gParentProcess() {
    if (gCurrentProcess) return ProcessManager::get().getprocess(gCurrentProcess->ppid);
    return nullptr;
}

static time_tick_callback_t::yield_vote_t tick_for_schedule(InterruptStack& stack, uint64_t, void*) {
    bool can_yield = ProcessManager::isinterruptible(stack.eip);
    bool must_yield = false;
    ProcessManager::get().tickForSchedule(can_yield, &must_yield);

    return must_yield ? time_tick_callback_t::yield : time_tick_callback_t::no_yield;
}

static time_tick_callback_t::yield_vote_t tick_for_metrics(InterruptStack&, uint64_t, void*) {
    ProcessManager::get().tickForMetrics();

    return time_tick_callback_t::no_yield;
}

namespace boot::proc_manager {
    uint32_t init() {
        ProcessManager &proc(ProcessManager::get());
        VirtualPageManager &vm(VirtualPageManager::get());
    	
        // one page for esp, one for esp0 and one for the process_t entry
        auto process_rgn_size = ProcessManager::gNumProcesses * VirtualPageManager::gPageSize * 3;
        interval_t rgn;
        if (false == vm.findKernelRegion(process_rgn_size, rgn)) {
            PANIC("cannot allocate memory for process objects");
        }
        vm.addKernelRegion(rgn.from, rgn.to);
        // vm.mapZeroPage(rgn.from, rgn.to);

        proc.mProcessPagesLow = rgn.from;
        proc.mProcessPagesHigh = rgn.to;

        LOG_DEBUG("process pages will live at [0x%p - 0x%p]", rgn.from, rgn.to);

        // enable real fault handlers
    	proc.installexceptionhandlers();
        LOG_INFO("real fault handlers enabled");

        return 0;
    }

    bool fail(uint32_t) {
        return bootphase_t::gPanic;
    }
}

namespace boot::scheduler_tick {
    uint32_t init() {
        auto& tmgr(TimeManager::get());

        tmgr.registerTickHandler(tick_for_schedule, nullptr, 5);
        tmgr.registerTickHandler(tick_for_metrics, nullptr, 1);

        return 0;
    }

    bool fail(uint32_t) {
        return bootphase_t::gPanic;
    }
}

exec_priority_t ProcessManager::gDefaultBasePriority {
    quantum : 10,
    scheduling : 20
};

ProcessManager& ProcessManager::get() {
    static ProcessManager gManager;

    return gManager;
}

#define PM_GLOBAL(Type, Name) \
Type & ProcessManager:: Name () { \
    static Type gRet; \
    return gRet; \
}

PM_GLOBAL(ProcessBitmap<ProcessManager::gNumProcesses>, gPidBitmap);
PM_GLOBAL(ProcessBitmap<ProcessManager::gNumProcesses>, gGDTBitmap);
PM_GLOBAL(ProcessTable<ProcessManager::gNumProcesses>, gProcessTable);
PM_GLOBAL(ProcessTable<ProcessManager::gNumProcesses>, gExitedProcesses);
PM_GLOBAL(slist<process_t*>, gCollectedProcessList);
PM_GLOBAL(vector<process_t*>, gReadyQueue);

#undef PM_GLOBAL

static process_t gDummyProcess;

static process_t *gSchedulerTask;
static process_t *gCollectorTask;
static process_t *gAwakerTask;
static process_t *gDeleterTask;
static process_t *gKeybQTask;
static process_t *gInitTask;

int ProcessManager::sleep_queue_helper::compare(const sleep_queue_helper::qentry& p1, const sleep_queue_helper::qentry& p2) {
    return p1.process->sleeptill >= p2.process->sleeptill ? -1 : +1;
}

pqueue<ProcessManager::sleep_queue_helper::qentry, ProcessManager::sleep_queue_helper>& ProcessManager::gSleepQueue() {
    static pqueue<ProcessManager::sleep_queue_helper::qentry, sleep_queue_helper> gRet;

    return gRet;
}

static void enqueueForCollection(process_t* task) {
    if (task->state != process_t::State::COLLECTED) {
        PANIC("only COLLECTED processes can go on collected list");
    }
    LOG_DEBUG("process %u going on collected list", task->pid);
    ProcessManager::gCollectedProcessList().add(task);
    tasks::deleter::queue().wakeall();
}

namespace {
    struct system_task_t {
        ProcessManager::spawninfo_t si;
        process_t **newProcess;
    };
}

#define NOSCHED 0
#define LOW 5
#define NORMAL 20
#define HIGH 30

#define SYSTEM_TASK(entry, sched, nm, dest) system_task_t { \
    si : ProcessManager::spawninfo_t { \
        cr3 : 0, \
        eip : (uintptr_t)&entry, \
        max_priority : exec_priority_t{ \
            quantum : (sched > 0) ? 1 : 0, \
            scheduling : sched, \
        }, \
        current_priority : exec_priority_t{ \
            quantum : (sched > 0) ? 1 : 0, \
            scheduling : sched, \
        }, \
        argument : 0, \
        environment : nullptr, \
        name : nm, \
        cwd : "/", \
        fileops : nullptr, \
        schedulable : sched, \
        system : true, \
    }, \
    newProcess : dest, \
}

static system_task_t gSystemTasks[] = {
    SYSTEM_TASK(tasks::scheduler::task,   NOSCHED,  "scheduler",   &gSchedulerTask),
    SYSTEM_TASK(tasks::collector::task,   LOW,      "collector",   &gCollectorTask),
    SYSTEM_TASK(tasks::awaker::task,      NORMAL,   "awaker",      &gAwakerTask),
    SYSTEM_TASK(tasks::deleter::task,     LOW,      "deleter",     &gDeleterTask),
    SYSTEM_TASK(tasks::keybqueue::task,   HIGH,     "keybqueue",   &gKeybQTask),
};

#undef SYSTEM_TASK
#undef NOSCHED
#undef LOW
#undef NORMAL
#undef HIGH

template<size_t N, typename T = system_task_t>
static void spawnSystemTasks(T (&tasks)[N]) {
    auto& pmm(ProcessManager::get());

    for(auto i = 0u; i < N; ++i) {
        const auto& task = tasks[i];
        auto process = pmm.kspawn(task.si);
        if (process) {
            *task.newProcess = process;
            LOG_INFO("spawned system process %s as 0x%p %u", task.si.name, process, (uint32_t)process->pid);
        } else {
            LOG_ERROR("failed to spawn system process %s", task.si.name);
            PANIC("system process setup failed");
        }
    }
}

extern "C"
void task0() {
    spawnSystemTasks(gSystemTasks);

    {
        auto&& pmm(ProcessManager::get());
        gInitTask = pmm.exec("/initrd/init", nullptr, nullptr, PROCESS_IS_FOREGROUND);
        gInitTask->flags.system = true;
        gInitTask->ttyinfo.tty->pushfg(gInitTask->pid);
        LOG_DEBUG("init process ready as 0x%p %u", gInitTask, (uint32_t)gInitTask->pid);
    }

    while(true) {
        haltforever();
    }
}

ProcessManager::ProcessManager() {
    {
        const auto num_reserved_gdt_selectors = val_numsysgdtentries<uint32_t>();
        auto& gdt_bitmap(gGDTBitmap());
        LOG_DEBUG("system reserved %u GDT selectors", num_reserved_gdt_selectors);
        for (auto i = 0u; i < num_reserved_gdt_selectors; ++i) {
            gdt_bitmap.reserve(i);
        }
    }

    static TTY gDummyProcessTTY;

    // prepare the initial dummy task
    gDummyProcess.tss.cr3 = readcr3();
    gDummyProcess.tss.eip = (uintptr_t)&task0;
    gDummyProcess.tss.cs = 0x0b;
    gDummyProcess.tss.ss = gDummyProcess.tss.ds = gDummyProcess.tss.es =
        gDummyProcess.tss.fs = gDummyProcess.tss.gs = 0x13;
    gDummyProcess.tss.ss0 = 0x10;
    gDummyProcess.tss.esp0 = 4096 + (uintptr_t)malloc(4096);
    gDummyProcess.pid = gPidBitmap().next();
    gDummyProcess.state = process_t::State::AVAILABLE;
    gDummyProcess.priority.quantum.current = gDummyProcess.priority.quantum.max = 1;
    gDummyProcess.path = strdup("kernel task");
    gDummyProcess.gdtidx = gGDTBitmap().next();
    gDummyProcess.ttyinfo = process_t::ttyinfo_t(&gDummyProcessTTY);
    gDummyProcess.flags.system = true;
    gDummyProcess.priority.scheduling.current = gDummyProcess.priority.quantum.current = 1;
    gDummyProcess.priority.scheduling.max = gDummyProcess.priority.quantum.max = 128;

    auto dtbl = addr_gdt<uint64_t*>();

    LOG_DEBUG("dtbl = 0x%p, sizeof(TSS) = %u, gDummyTaskSegment = 0x%p", dtbl, sizeof(process_t), &gDummyProcess);
    LOG_DEBUG("msb is 0x%p", ((uint64_t)&gDummyProcess) & 0xFF000000);

    dtbl[gDummyProcess.gdtidx] = sizeof(process_t) & 0xFFFF;
    dtbl[gDummyProcess.gdtidx] |= (((uint64_t)&gDummyProcess) & 0xFFFFFFULL) << 16;
    dtbl[gDummyProcess.gdtidx] |= 0xE90000000000ULL;
    dtbl[gDummyProcess.gdtidx] |= (((uint64_t)&gDummyProcess) & 0xFF000000ULL) << 32;

    LOG_DEBUG("gdt idx is %u entry is 0x%llx", gDummyProcess.gdtidx, dtbl[gDummyProcess.gdtidx]);

    dtbl[5] = 0x300000 | 0x1E50000000000;
    writetaskreg(gDummyProcess.gdtidx*8 | 3);

    gCurrentProcess = &gDummyProcess;
    gProcessTable().set(&gDummyProcess);
    gReadyQueue().push_back(&gDummyProcess);

    mProcessPagesLow = mProcessPagesHigh = 0;
}

process_t* ProcessManager::exec(const char* path, const char** args, const char** env, uint32_t flags, exec_priority_t prio, uintptr_t argp, exec_fileop_t* fops) {
    auto&& vmm(VirtualPageManager::get());

    spawninfo_t si {
        cr3 : vmm.createAddressSpace(),
        eip : (uintptr_t)&fileloader,
        max_priority : exec_priority_t {
            quantum : gCurrentProcess->priority.quantum.max,
            scheduling : gCurrentProcess->priority.scheduling.max,
        },
        current_priority : prio,
        argument : argp,
        environment : env,
        name : path,
        cwd : (flags & PROCESS_INHERITS_CWD ? gCurrentProcess->cwd : "/"),
        fileops : fops,
        schedulable : true,
        system : (flags & PROCESS_INHERITS_SYSTEM ? gCurrentProcess->flags.system : false),
        clone : false
    };

    if (auto process = spawn(si)) {
        if (args) process->copyArguments(args, false);
        return process;
    }

    return nullptr;
}

template<int16_t idx>
static typename enable_if<idx >= 0, uint32_t*>::type stackpush(uint32_t* esp) {
    return &esp[idx];
}

template<int16_t idx, typename T = uint32_t, typename... Q>
static typename enable_if<idx >= 0, uint32_t*>::type stackpush(uint32_t *esp, T value, Q... values) {
    esp[idx] = value;
    return stackpush<idx-1>(esp, values...);
}

namespace {
    class process_pages_t {
        public:
            const uintptr_t proc;
            const uintptr_t esp0;
            const uintptr_t esp;

            static process_pages_t find(uintptr_t low, uintptr_t high, bool& ok) {
                auto mapopts = VirtualPageManager::map_options_t().clear(true).user(false).rw(true);
                auto&& vm(VirtualPageManager::get());

                auto pp = vm.mapPageWithinRange(low, high, mapopts);
                auto e0 = vm.mapPageWithinRange(low, high, mapopts);
                auto ep = vm.mapPageWithinRange(low, high, mapopts);

                if (pp & 1) {
                    ok = false;
                } else if (e0 & 1) {
                    ok = false;
                } else if (ep & 1) {
                    ok = false;
                } else {
                    ok = true;
                }

                return process_pages_t(pp, e0, ep);
            }

            template<typename T = uint32_t>
            T* getesp() {
                return (T*)esp;
            }

            template<typename T = uint32_t>
            T* getesp0() {
                return (T*)esp0;
            }

            process_t *makeProcess() {
                process_t *task = new ((void*)proc) process_t();

                task->espstart = esp;
                task->esp0start = esp0;

                return task;
            }

        private:
            process_pages_t(uintptr_t proc, uintptr_t esp0, uintptr_t esp) : proc(proc), esp0(esp0), esp(esp) {}
    };
}

size_t ProcessManager::numProcesses() {
    return gProcessTable().size();
}

void ProcessManager::foreach(function<bool(const process_t*)> f) {
    gProcessTable().foreach(f);
}

static char** copyEnvironment(process_t* dest, const char** env) {
    size_t envs = 0;
    while(env != nullptr) {
        if (env[envs] == nullptr) break;
        ++envs;
    }

    dest->environ = (char**)malloc(sizeof(char*) * (envs + 1));
    dest->environ[envs] = nullptr;
    for (auto i = 0u; i < envs; ++i) {
        dest->environ[i] = strdup(env[i]);
    }

    return dest->environ;
}

process_t* ProcessManager::spawn(const spawninfo_t& si) {
    if (mProcessPagesLow == 0 || mProcessPagesHigh == 0) {
        PANIC("set process table location before spawning");
    }
    if (si.cr3 == 0) {
        PANIC("set a valid non-zero CR3 before spawning");
    }

    if (gCurrentProcess) {
        if (!gCurrentProcess->flags.system) {
            const bool too_high_quantum = si.max_priority.quantum > gCurrentProcess->priority.quantum.max;
            const bool too_high_scheduling = si.max_priority.scheduling > gCurrentProcess->priority.scheduling.max;
            const bool too_priority = too_high_quantum || too_high_scheduling;

            const bool out_of_range_quantum = si.current_priority.quantum > si.max_priority.quantum;
            const bool out_of_range_scheduling = si.current_priority.scheduling > si.max_priority.scheduling;
            const bool out_of_range = out_of_range_quantum || out_of_range_scheduling;
            if (too_priority) {
                if (too_high_quantum) {
                    LOG_ERROR("quantum %u is higher than maximum allowed %u", si.max_priority.quantum, gCurrentProcess->priority.quantum.max);
                } else {
                    LOG_ERROR("scheduling priority %llu is higher than maximum allowed %llu", si.max_priority.scheduling, gCurrentProcess->priority.scheduling.max);
                }
                return nullptr;
            } else if (out_of_range) {
                if (out_of_range_quantum) {
                    LOG_ERROR("quantum %u is higher than maximum allowed %u", si.current_priority.quantum, si.max_priority.quantum);
                } else {
                    LOG_ERROR("scheduling priority %llu is higher than maximum allowed %llu", si.current_priority.scheduling, si.max_priority.scheduling);
                }
                return nullptr;
            }
        }
    }

    bool ok = false;
    auto pages = process_pages_t::find(mProcessPagesLow, mProcessPagesHigh, ok);
    if (!ok) {
        PANIC("unable to find memory for a new process");
    }
    auto process = pages.makeProcess();
    LOG_DEBUG("setup new process_t page at 0x%p", process);

    if (si.clone) {
        LOG_DEBUG("new process is cloned");
        gCurrentProcess->clone(process);
    } else {
        copyEnvironment(process, si.environment);
    }

    if (si.name) process->path = strdup(si.name);

    process->tss.cr3 = si.cr3;
    process->tss.eip = si.eip;
    process->pid = gPidBitmap().next();
    process->ppid = gCurrentProcess->pid;
    process->ttyinfo = gCurrentProcess->ttyinfo;
    process->cwd = strdup(gCurrentProcess->cwd);

    process->priority.quantum.max = si.max_priority.quantum;
    process->priority.scheduling.max = si.max_priority.scheduling;
    process->priority.quantum.current = si.current_priority.quantum;
    process->priority.scheduling.current = si.current_priority.scheduling;

    process->flags.system = si.system;

    process->tss.cs = 0x08;
    process->tss.ss = process->tss.ds = process->tss.es = process->tss.fs = process->tss.gs = 0x10;
    process->tss.ss0 = 0x10;
    process->tss.esp0 = VirtualPageManager::gPageSize - 1 + pages.esp0;

    process->tss.eflags = 512; // enable IRQ

    // TODO: is one page enough? factor this out to a global anyway
    // this is [0] thru [1023], where [1023] is the first element
    // because of x86 stack rules
    uint32_t* esp = pages.getesp();
    esp = stackpush<1023>(esp, si.argument);
    process->tss.esp = (uintptr_t)esp;

    LOG_DEBUG("esp = 0x%p", process->tss.esp);

    auto gdtval = fillGDT(process);

    LOG_DEBUG("process %u spawning process %u; eip = 0x%p, cr3 = 0x%p, esp0 = 0x%p, esp = 0x%p, gdt index %u value 0x%llx",
        process->ppid, process->pid,
        process->tss.eip, process->tss.cr3,
        process->tss.esp0, process->tss.esp,
        process->gdtidx, gdtval);

    gProcessTable().set(process);
    if (si.schedulable) {
        LOG_DEBUG("process %u is schedulable", process->pid);
        reschedule(process);
    } else {
        LOG_DEBUG("process %u is not schedulable", process->pid);
    }

    forwardTTY(process);

    execFileops(gCurrentProcess, process, si.fileops);

    gCurrentProcess->children.add(process);

    return process;
}

process_t* ProcessManager::kspawn(const spawninfo_t& si) {
    auto sinfo(si);
    sinfo.cr3 = gDummyProcess.tss.cr3;
    sinfo.system = true;
    return spawn(sinfo);
}

#define GDT_IDX_TO_TSS_ENTRY(idx) (((idx * 8) << 16) | 0x1E50000000000ULL)

static uint64_t gNumCtxSwitches = 0;

uint64_t ProcessManager::numContextSwitches() {
    return gNumCtxSwitches;
}

static inline void doGDTSwitch(uint16_t, size_t gdtidx) {
    ++gNumCtxSwitches;
    auto dtbl = addr_gdt<uint64_t*>();
    dtbl[5] = GDT_IDX_TO_TSS_ENTRY(gdtidx);
    ::ctxswitch();
}

void ProcessManager::ctxswitch(process_t* task) {
    auto cr0 = gCurrentProcess->cr0;
    if (0 == (cr0 & 0x8)) {
        LOG_DEBUG("saving FP state for process %u", gCurrentProcess->pid);
        cleartaskswitchflag();
        fpsave((uintptr_t)&gCurrentProcess->fpstate[0]);
    }

    gCurrentProcess = task;
    doGDTSwitch(task->pid, task->gdtidx);
}

void ProcessManager::switchtoscheduler() {
    // cr0 is not part of the hardware context switch, save it upon switching to scheduler
    gCurrentProcess->cr0 = readcr0();
    doGDTSwitch(gSchedulerTask->pid, gSchedulerTask->gdtidx);
}

#define GDT_ENTRY_TO_TSS_PTR(tssval) ((tssval & 0xFFFFFF0000ULL) >> 16) | ((tssval & 0xFF00000000000000ULL) >> 32)

process_t* ProcessManager::getprocess(kpid_t pid) {
    return gProcessTable().get(pid);
}

#undef GDT_ENTRY_TO_TSS_PTR

kpid_t ProcessManager::getpid() {
    return gCurrentProcess->pid;
}

void ProcessManager::sleep(uint32_t durationMs) {
    gCurrentProcess->sleeptill = TimeManager::get().millisUptime() + durationMs;
    LOG_DEBUG("task %u scheduled to sleep till %llu", gCurrentProcess->pid, gCurrentProcess->sleeptill);
    deschedule(gCurrentProcess, process_t::State::SLEEPING, nullptr);
    gSleepQueue().insert({gCurrentProcess->waitToken, gCurrentProcess});
    tasks::awaker::queue().wakeall();
    yield();
}

// This is a fairly sharp tool: it takes a (non-running) process and hijacks it
// into going to a whole different place next time it's scheduled. This has the potential
// to lose all information about where the process originally was and what it was doing.
void ProcessManager::resumeat(process_t* task, uintptr_t eip, uintptr_t esp, uint16_t cs, uint16_t ss) {
    auto&& tss(task->tss);

    tss.eip = eip;
    tss.esp = esp;
    tss.cs = cs;
    tss.ss = ss;
}

bool ProcessManager::kill(kpid_t pid) {
    process_exit_status_t es(process_exit_status_t::reason_t::killed, 0);
    auto task = getprocess(pid);
    LOG_DEBUG("task %u 0x%p killing task %u 0x%p", gCurrentProcess->pid, gCurrentProcess, task->pid, task);
    if (task == gCurrentProcess) {
        exit(es);
    } else if (task != nullptr) {
        if (task->flags.system && !gCurrentProcess->flags.system) {
            LOG_ERROR("process %u tried to kill system task %u", gCurrentProcess->pid, task->pid);
            return false;
        }
        // push the exit status
        uint32_t *stack = (uint32_t*)task->esp0start;
        *stack = es.toWord();
        --stack;
        resumeat(task, (uintptr_t)&reaper, (uint32_t)stack, 0x08, 0x10);
    }
    return true;
}

void ProcessManager::exit(process_exit_status_t es) {
    exit(gCurrentProcess, es);
    yield();
}

void ProcessManager::exit(process_t* task, process_exit_status_t es) {
    if (task->flags.system) {
        PANIC("attempting to kill a system process");
    } else {
        LOG_DEBUG("killing process %u", task->pid);
    }

    {
        auto c0 = task->children.begin();
        auto end = task->children.end();

        while (c0 != end) {
            LOG_DEBUG("child %u going to collector process %u", (*c0)->pid, gCollectorTask->pid);
            (*c0)->ppid = gCollectorTask->pid;
            gCollectorTask->children.add(*c0);
            ++c0;
            tasks::collector::queue().wakeall();
        }

        task->children.clear();
    }
    LOG_DEBUG("done reparenting processes");

    task->getMemoryManager()->cleanupAllRegions();
    LOG_DEBUG("done cleaning memory regions");
    VirtualPageManager::get().cleanAddressSpace();

    for(auto i = 0u; i < task->fds.size(); ++i) {
        decltype(task->fds)::entry_t fd;
        if (task->fds.is(i, &fd)) {
            if (fd) {
                LOG_DEBUG("for process %u, closing file handle %u (fs = 0x%p, fh = 0x%p)", task->pid, i, fd.filesystem, fd.object);
                fd.close();
            }
        }
    }
    LOG_DEBUG("done releasing file handles");

    task->state = process_t::State::EXITED;
    task->ttyinfo.tty->popfg(task->pid);
    task->exitstatus = es;

    auto parent = getprocess(task->ppid);
    if (parent == nullptr) {
        LOG_DEBUG("process has no parent");
    }
    if (parent->state == process_t::State::COLLECTING) {
        LOG_DEBUG("parent %u woken up for collection", parent->pid);
        // TODO: a process should be a WaitableObject
        reschedule(parent);
    } else {
        LOG_DEBUG("parent %u was not waiting to collect", parent->pid);
    }
}

bool ProcessManager::isinterruptible(uintptr_t addr) {
    static uintptr_t gHaltInstruction = (uintptr_t)&haltforever;
    // the EIP value at *hlt* will be &hlt + 1
    static uintptr_t gHaltEIP = gHaltInstruction + 1;

    if (VirtualPageManager::iskernel(addr)) {
        return ((addr == gHaltInstruction) || (addr == gHaltEIP));
    } else {
        return true;
    }
}

void ProcessManager::tickForMetrics() {
    if (gCurrentProcess) __sync_add_and_fetch(&gCurrentProcess->runtimestats.runtime, TimeManager::get().millisPerTick());
}

void ProcessManager::tickForSchedule(bool can_yield, bool* will_yield) {
    *will_yield = false;

    auto allowedticks = __atomic_load_n(&gCurrentProcess->priority.quantum.current, __ATOMIC_SEQ_CST);
    if (allowedticks > 0) {
        auto usedticks = __atomic_add_fetch(&gCurrentProcess->usedticks, 1, __ATOMIC_SEQ_CST);
        if (usedticks >= allowedticks) {
            if (can_yield) *will_yield = true;
            else gCurrentProcess->flags.due_for_reschedule = true;
        }
    }
}

void ProcessManager::yield(bool bytimer) {
    if (gCurrentProcess) {
        __sync_add_and_fetch(&gCurrentProcess->runtimestats.ctxswitches, 1);
        if (!bytimer) __sync_add_and_fetch(&gCurrentProcess->runtimestats.runtime, 1);
    }
    switchtoscheduler();
}

kpid_t ProcessManager::initpid() {
    return gInitTask->pid;
}
kpid_t schedulerpid() {
    return gSchedulerTask->pid;
}

void ProcessManager::ready(process_t* task, void* waitable) {
    if (task->state == process_t::State::AVAILABLE) {
        LOG_INFO("waking up task %u which is already awake", task->pid);
    } else {
        task->wakeReason.timeout = false;
        task->wakeReason.waitable = waitable;
        reschedule(task);
    }
}

void ProcessManager::wake(process_t* task) {
    if (task->state == process_t::State::AVAILABLE) {
        LOG_INFO("waking up task %u which is already awake", task->pid);
    } else {
        if (task->wakeReason.waitable) {
            ((WaitQueue*)task->wakeReason.waitable)->remove(task);
        }
        task->wakeReason.timeout = true;
        task->wakeReason.waitable = nullptr;
        reschedule(task);
    }
}

void ProcessManager::reschedule(process_t* task) {
    task->waitToken += 1;
    task->state = process_t::State::AVAILABLE;
    gReadyQueue().push_back(task);
}

void ProcessManager::deschedule(process_t* task, process_t::State newstate, void* waitable) {
    int numfound = 0;

    auto&& rq(gReadyQueue());
    auto b = rq.begin(), e = rq.end();
    while(b != e) {
        if (*b == task) {
            ++numfound;
            (*b)->state = newstate;
            (*b)->wakeReason.waitable = waitable;
            rq.erase(b);
            b = rq.begin();
            e = rq.end();
        } else {
            ++b;
        }
    }

    if (numfound > 1) {
        PANIC("process enqueued for execution multiple times");
    }
}

void ProcessManager::enqueueForDeath(process_t* task) {
    if (task->state != process_t::State::EXITED) {
        PANIC("only EXITED processes can go on exited list");
    }
    LOG_DEBUG("process %u going on exited list", task->pid);
    gExitedProcesses().set(task);
}

bool ProcessManager::collectany(bool wait, kpid_t* pid, process_exit_status_t* status) {
    do {
        auto c0 = gCurrentProcess->children.begin();
        auto ce = gCurrentProcess->children.end();

        for (; c0 != ce; ++c0) {
            auto child = *c0;
            if (child->state == process_t::State::EXITED) {
                *status = collect(*pid = child->pid);
                LOG_DEBUG("process %u collectany'd by parent %u", *pid, gCurrentProcess->pid);
                return true;
            }
        }
        if (wait) {
            deschedule(gCurrentProcess, process_t::State::COLLECTING, nullptr);
            yield();
        }
    } while(wait);

    // no child has EXITED yet
    return false;
}

process_exit_status_t ProcessManager::collect(kpid_t pid) {
begin:
    auto task = getprocess(pid);

    LOG_DEBUG("process %u (0x%p) trying to collect process %u (0x%p)", gCurrentProcess->pid, gCurrentProcess, pid, task);

    if (task == nullptr) {
        // TODO: make this a fatal process error
        PANIC("cannot collect a non-existant process");
    }

    auto parent = getprocess(task->ppid);

    if (parent != gCurrentProcess) {
        // TODO: make this a fatal process error
        PANIC("cannot collect another process' child - that's creepy");
    }

    if (task->state != process_t::State::EXITED) {
        // cannot collect a non-EXITED process
        LOG_DEBUG("attempting to collect task %u, not dead yet - parent %u must wait", task->pid, gCurrentProcess->pid);
        deschedule(gCurrentProcess, process_t::State::COLLECTING, nullptr);
        yield();
        goto begin;
    }

    LOG_DEBUG("task %u collecting dead task %u", gCurrentProcess->pid, task->pid);

    bool found = false;
    auto c0 = parent->children.begin();
    auto ce = parent->children.end();
    for (; c0 != ce; ++c0) {
        LOG_DEBUG("scanning children of parent %u, found 0x%p %u", gCurrentProcess->pid, *c0, (*c0)->pid);
        if ((*c0) == task) {
            parent->children.remove(c0);
            found = true;
            break;
        }
    }

    if (!found) {
        PANIC("process not listed in children of parent");
    }

    auto result = task->exitstatus;

    gExitedProcesses().free(task);

    task->state = process_t::State::COLLECTED;
    enqueueForCollection(task);

    return result;
}

process_t* ProcessManager::cloneProcess(uintptr_t eip, exec_fileop_t* fops) {
    auto&& vm(VirtualPageManager::get());

    spawninfo_t si {
        cr3 : vm.cloneAddressSpace(),
        eip : (uintptr_t)clone_start,
        max_priority : exec_priority_t {
            quantum : gCurrentProcess->priority.quantum.max,
            scheduling : gCurrentProcess->priority.scheduling.max,
        },
        current_priority : exec_priority_t {
            quantum : gCurrentProcess->priority.quantum.current,
            scheduling : gCurrentProcess->priority.scheduling.current,
        },
        argument : eip,
        environment : nullptr,
        name : gCurrentProcess->path,
        cwd : gCurrentProcess->cwd,
        fileops : fops,
        schedulable : true,
        system : gCurrentProcess->flags.system,
        clone : true
    };

    return spawn(si);
}

uint64_t ProcessManager::fillGDT(process_t* process) {
    auto dtbl = addr_gdt<uint64_t*>();

    process->gdtidx = gGDTBitmap().next();

    dtbl[process->gdtidx] = sizeof(process_t) & 0xFFFF;
    dtbl[process->gdtidx] |= (((uint64_t)process) & 0xFFFFFFULL) << 16;
    dtbl[process->gdtidx] |= 0xE90000000000ULL;
    dtbl[process->gdtidx] |= (((uint64_t)process) & 0xFF000000ULL) << 32;

    return dtbl[process->gdtidx];
}

void ProcessManager::forwardTTY(process_t* process) {
    size_t ttyfd0=3, ttyfd1=3, ttyfd2=3;
    bool ok0 = process->fds.set({nullptr, &process->ttyinfo.ttyfile}, ttyfd0);
    bool ok1 = process->fds.set({nullptr, &process->ttyinfo.ttyfile}, ttyfd1);
    bool ok2 = process->fds.set({nullptr, &process->ttyinfo.ttyfile}, ttyfd2);
    if ((ok0 && 0 == ttyfd0) && (ok1 && 1 == ttyfd1) && (ok2 && 2 == ttyfd2)) {
        LOG_DEBUG("TTY setup complete - tty is 0x%p ttyfile is 0x%p", process->ttyinfo.tty, &process->ttyinfo.ttyfile);
    } else {
        PANIC("unable to forward TTY to new process");
    }
}

void ProcessManager::execFileops(process_t* parent, process_t* child, exec_fileop_t *fops) {
    if (fops == nullptr) return;
    while(fops->op != exec_fileop_t::operation::END_OF_LIST) {
        switch (fops->op) {
            case exec_fileop_t::operation::CLOSE_CHILD_FD: {
                auto child_fd = fops->param1;
                VFS::filehandle_t child_handle = {nullptr, nullptr};
                bool ok = child->fds.is(child_fd, &child_handle);
                if (ok) {
                    if (child_handle) {
                        child_handle.close();
                    }
                }
                child->fds.clear(child_fd);
                TAG_DEBUG(FILEOPS, "closed handle %u in child process %u", child_fd, child->pid);
            }
                break;
            case exec_fileop_t::operation::DUP_PARENT_FD: {
                auto parent_fd = fops->param1;
                VFS::filehandle_t parent_handle = {nullptr, nullptr};
                bool ok = parent->fds.is(parent_fd, &parent_handle);
                if (!ok) {
                    TAG_ERROR(FILEOPS, "file handle %u in parent process %u is not available; can't duplicate", parent_fd, parent->pid);
                } else {
                    if (parent_handle.object) {
                        parent_handle.object->incref();
                        ok = child->fds.set(parent_handle, fops->param2);
                        if (!ok) {
                            TAG_ERROR(FILEOPS, "failed to duplicate handle %u from process %u in child process %u",
                                parent_fd, parent->pid, child->pid);
                        } else {
                            TAG_DEBUG(FILEOPS, "handle %u from parent %u will be handle %u in child %u",
                                parent_fd, parent->pid, fops->param2, child->pid);
                        }
                    }
                }
            }
                break;
            case exec_fileop_t::operation::DUP_CHILD_FD:
                TAG_ERROR(FILEOPS, "DUP_CHILD_FD is not implemented yet");
                break;
            default:
                LOG_WARNING("unknown fileop value: %u", fops->op);
                break;
        }
        ++fops;
    }
}
