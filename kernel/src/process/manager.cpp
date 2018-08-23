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
#include <kernel/drivers/pit/pit.h>
#include <kernel/libc/slist.h>
#include <kernel/process/process.h>
#include <kernel/mm/virt.h>
#include <kernel/process/fileloader.h>
#include <kernel/process/clonestart.h>
#include <kernel/process/reaper.h>
#include <kernel/boot/phase.h>
#include <kernel/libc/vec.h>
#include <kernel/synch/semaphoremanager.h>
#include <kernel/synch/mutexmanager.h>
#include <kernel/tasks/scheduler.h>
#include <kernel/tasks/awaker.h>
#include <kernel/tasks/collector.h>
#include <kernel/tasks/deleter.h>

LOG_TAG(TIMING, 2);

// this is THE current process information
process_t *gCurrentProcess;

namespace boot::task {
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

        LOG_DEBUG("process pages will live at [%p - %p]", rgn.from, rgn.to);

        // enable real fault handlers
    	proc.installexceptionhandlers();
        LOG_INFO("real fault handlers enabled");

        return 0;
    }

    bool fail(uint32_t) {
        return bootphase_t::gPanic;
    }
}

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
static process_t *gInitTask;

int ProcessManager::processcomparator::compare(process_t* p1, process_t* p2) {
    return p1->sleeptill >= p2->sleeptill ? -1 : +1;
}

pqueue<process_t*, ProcessManager::processcomparator>& ProcessManager::gSleepQueue() {
    static pqueue<process_t*, processcomparator> gRet;

    return gRet;
}

static void enqueueForCollection(process_t* task) {
    if (task->state != process_t::State::COLLECTED) {
        PANIC("only COLLECTED processes can go on collected list");
    }
    LOG_DEBUG("process %u going on collected list", task->pid);
    ProcessManager::gCollectedProcessList().add(task);
}

namespace {
    struct system_task_t {
        ProcessManager::spawninfo_t si;
        process_t **newProcess;
    };
}

#define SYSTEM_TASK(entry, sched, nm, dest) system_task_t { \
    si : ProcessManager::spawninfo_t { \
        cr3 : 0, \
        eip : (uintptr_t)&entry, \
        priority : sched ? 1 : 0, \
        argument : 0, \
        name : nm, \
        schedulable : sched, \
        system : true, \
    }, \
    newProcess : dest, \
}

static system_task_t gSystemTasks[] = {
    SYSTEM_TASK(tasks::scheduler::task, false, "scheduler", &gSchedulerTask),
    SYSTEM_TASK(tasks::collector::task, true,  "collector", &gCollectorTask),
    SYSTEM_TASK(tasks::awaker::task,    true,  "awaker",    &gAwakerTask),
    SYSTEM_TASK(tasks::deleter::task,   true,  "deleter",   &gDeleterTask),
};

#undef SYSTEM_TASK

template<size_t N, typename T = system_task_t>
static void spawnSystemTasks(T (&tasks)[N]) {
    auto& pmm(ProcessManager::get());

    for(auto i = 0u; i < N; ++i) {
        const auto& task = tasks[i];
        auto process = pmm.kspawn(task.si);
        if (process) {
            *task.newProcess = process;
            LOG_INFO("spawned system process %s as %p %u", task.si.name, process, (uint32_t)process->pid);
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
        gInitTask = pmm.setup("/initrd/init", nullptr);
        gInitTask->flags.system = true;
        gInitTask->ttyinfo.tty->pushfg(gInitTask->pid);
        LOG_DEBUG("init process ready as %p %u", gInitTask, (uint32_t)gInitTask->pid);
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
    static TTYFile gDummyProcessTTYFile(&gDummyProcessTTY);

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
    gDummyProcess.priority.prio = gDummyProcess.priority.prio0 = 1;
    gDummyProcess.path = strdup("kernel task");
    gDummyProcess.gdtidx = gGDTBitmap().next();
    gDummyProcess.ttyinfo = process_t::ttyinfo_t(&gDummyProcessTTY, &gDummyProcessTTYFile);

    auto dtbl = addr_gdt<uint64_t*>();

    LOG_DEBUG("dtbl = %p, sizeof(TSS) = %u, gDummyTaskSegment = %p", dtbl, sizeof(process_t), &gDummyProcess);
    LOG_DEBUG("msb is %p", ((uint64_t)&gDummyProcess) & 0xFF000000);

    dtbl[gDummyProcess.gdtidx] = sizeof(process_t) & 0xFFFF;
    dtbl[gDummyProcess.gdtidx] |= (((uint64_t)&gDummyProcess) & 0xFFFFFFULL) << 16;
    dtbl[gDummyProcess.gdtidx] |= 0xE90000000000ULL;
    dtbl[gDummyProcess.gdtidx] |= (((uint64_t)&gDummyProcess) & 0xFF000000ULL) << 32;

    LOG_DEBUG("gdt idx is %u entry is %llx", gDummyProcess.gdtidx, dtbl[gDummyProcess.gdtidx]);

    dtbl[5] = 0x300000 | 0x1E50000000000;
    writetaskreg(gDummyProcess.gdtidx*8 | 3);

    gCurrentProcess = &gDummyProcess;
    gProcessTable().set(&gDummyProcess);
    gReadyQueue().push_back(&gDummyProcess);

    mProcessPagesLow = mProcessPagesHigh = 0;
}

process_t* ProcessManager::setup(const char* path, const char* args, uint8_t prio, uintptr_t argp) {
    auto&& vmm(VirtualPageManager::get());

    spawninfo_t si {
        cr3 : vmm.createAddressSpace(),
        eip : (uintptr_t)&fileloader,
        priority : prio,
        argument : argp,
        name : path,
        schedulable : true,
    };

    if (auto process = spawn(si)) {
        if (args) process->args = strdup(args);
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

process_t* ProcessManager::spawn(const spawninfo_t& si) {
    if (mProcessPagesLow == 0 || mProcessPagesHigh == 0) {
        PANIC("set process table location before spawning");
    }
    if (si.cr3 == 0) {
        PANIC("set a valid non-zero CR3 before spawning");
    }

    bool ok = false;
    auto pages = process_pages_t::find(mProcessPagesLow, mProcessPagesHigh, ok);
    if (!ok) {
        PANIC("unable to find memory for a new process");
    }
    auto process = pages.makeProcess();
    LOG_DEBUG("setup new process_t page at %p", process);

    if (si.clone) {
        LOG_DEBUG("new process is cloned");
        gCurrentProcess->clone(process);
    }

    if (si.name) process->path = strdup(si.name);

    process->tss.cr3 = si.cr3;
    process->tss.eip = si.eip;
    process->pid = gPidBitmap().next();
    process->ppid = gCurrentProcess->pid;
    process->ttyinfo = gCurrentProcess->ttyinfo;
    process->priority.prio = process->priority.prio0 = si.priority;
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

    LOG_DEBUG("esp = %p", process->tss.esp);

    auto gdtval = fillGDT(process);

    LOG_DEBUG("process %u spawning process %u; eip = %p, cr3 = %p, esp0 = %p, esp = %p, gdt index %u value %llx",
        process->ppid, process->pid,
        process->tss.eip, process->tss.cr3,
        process->tss.esp0, process->tss.esp,
        process->gdtidx, gdtval);

    gProcessTable().set(process);
    if (si.schedulable) {
        LOG_DEBUG("process %u is schedulable", process->pid);
        gReadyQueue().push_back(process);
        process->state = process_t::State::AVAILABLE;
    } else {
        LOG_DEBUG("process %u is not schedulable", process->pid);        
        process->state = process_t::State::NEW;        
    }

    forwardTTY(process);

    gCurrentProcess->children.add(process);

    return process;
}

process_t* ProcessManager::kspawn(const spawninfo_t& si) {
    auto sinfo(si);
    sinfo.cr3 = gDummyProcess.tss.cr3;
    sinfo.system = true;
    return spawn(sinfo);
}

#define PID_TO_TSS_ENTRY(pid) ((((pid + 6)*8) << 16) | 0x1E50000000000ULL)

static inline void doGDTSwitch(uint16_t pid) {
    auto dtbl = addr_gdt<uint64_t*>();
    dtbl[5] = PID_TO_TSS_ENTRY(pid);
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
    doGDTSwitch(task->pid);
}

void ProcessManager::switchtoscheduler() {
    // cr0 is not part of the hardware context switch, save it upon switching to scheduler
    gCurrentProcess->cr0 = readcr0();
    doGDTSwitch(gSchedulerTask->pid);
}

void ProcessManager::ctxswitch(pid_t task) {
    ctxswitch(getprocess(task));
}

#define GDT_ENTRY_TO_TSS_PTR(tssval) ((tssval & 0xFFFFFF0000ULL) >> 16) | ((tssval & 0xFF00000000000000ULL) >> 32)

process_t* ProcessManager::getprocess(ProcessManager::pid_t pid) {
    return gProcessTable().get(pid);
}

#undef GDT_ENTRY_TO_TSS_PTR

ProcessManager::pid_t ProcessManager::getpid() {
    return gCurrentProcess->pid;
}

void ProcessManager::sleep(uint32_t durationMs) {
    gCurrentProcess->sleeptill = PIT::getUptime() + durationMs;
    LOG_DEBUG("task %u scheduled to sleep till %llu", gCurrentProcess->pid, gCurrentProcess->sleeptill);
    deschedule(gCurrentProcess, process_t::State::SLEEPING);
    gSleepQueue().insert(gCurrentProcess);
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

void ProcessManager::kill(pid_t pid) {
    process_exit_status_t es(process_exit_status_t::reason_t::killed, 0);
    auto task = getprocess(pid);
    LOG_DEBUG("task %u %p killing task %u %p", gCurrentProcess->pid, gCurrentProcess, task->pid, task);
    if (task == gCurrentProcess) {
        exit(es);
    } else if (task != nullptr) {
        // push the exit status
        uint32_t *stack = (uint32_t*)task->esp0start;
        *stack = es.toWord();
        --stack;
        resumeat(task, (uintptr_t)&reaper, (uint32_t)stack, 0x08, 0x10);
    }
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
        }

        task->children.clear();
    }
    LOG_DEBUG("done reparenting processes");

    for(auto i = 0u; i < task->fds.size(); ++i) {
        decltype(task->fds)::entry_t fd;
        if (task->fds.is(i, &fd)) {
            if (fd.first && fd.second) {
                LOG_DEBUG("for process %u, closing file handle %u (fs = %p, fh = %p)", task->pid, i, fd.first, fd.second);
                fd.first->close(fd.second);
            }
        }
    }
    LOG_DEBUG("done releasing file handles");

    {
        auto& semgr(SemaphoreManager::get());

        for(auto i = 0u; i < task->semas.size(); ++i) {
            decltype(task->semas)::entry_t sd;
            if (task->semas.is(i, &sd) && sd) {
                LOG_DEBUG("for process %u, releasing semaphore %u (sd = %p)", task->pid, i, sd);
                semgr.release(sd);
            }
        }
    }
    LOG_DEBUG("done releasing semaphores");

    {
        auto& mtxmgr(MutexManager::get());

        for(auto i = 0u; i < task->mutexes.size(); ++i) {
            decltype(task->mutexes)::entry_t sd;
            if (task->mutexes.is(i, &sd) && sd) {
                LOG_DEBUG("for process %u, releasing mutex %u (sd = %p)", task->pid, i, sd);
                mtxmgr.release(sd);
            }
        }
    }
    LOG_DEBUG("done releasing mutexes");

    task->state = process_t::State::EXITED;
    task->ttyinfo.tty->popfg();
    task->ttyinfo.tty->resetEOF();
    task->exitstatus = es;

    auto parent = getprocess(task->ppid);
    if (parent == nullptr) {
        LOG_DEBUG("process has no parent");
    }
    if (parent->state == process_t::State::COLLECTING) {
        LOG_DEBUG("parent %u woken up for collection", parent->pid);
        ready(parent);
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

void ProcessManager::tick() {
    auto allowedticks = __atomic_load_n(&gCurrentProcess->priority.prio, __ATOMIC_SEQ_CST);
    if (allowedticks > 0) {
        auto usedticks = __atomic_add_fetch(&gCurrentProcess->usedticks, 1, __ATOMIC_SEQ_CST);
        gCurrentProcess->runtimestats.runtime += PIT::gTickDuration;
        if (usedticks >= allowedticks) {
            yield(true);
        }
    }
}

void ProcessManager::yield(bool bytimer) {
    if (!bytimer) gCurrentProcess->runtimestats.runtime += PIT::gTickDuration / 2;
    switchtoscheduler();
}

process_t::pid_t ProcessManager::initpid() {
    return gInitTask->pid;
}
process_t::pid_t schedulerpid() {
    return gSchedulerTask->pid;
}

void ProcessManager::ready(process_t* task) {
    task->state = process_t::State::AVAILABLE;
    gReadyQueue().push_back(task);

    LOG_DEBUG("process %u moved to state AVAILABLE", task->pid);
}

void ProcessManager::deschedule(process_t* task, process_t::State newstate) {
    int numfound = 0;

    auto&& rq(gReadyQueue());
    auto b = rq.begin(), e = rq.end();
    while(b != e) {
        if (*b == task) {
            ++numfound;
            (*b)->state = newstate;
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

    LOG_DEBUG("process %u moved to state %u", task->pid, (uint8_t)newstate);
}

void ProcessManager::enqueueForDeath(process_t* task) {
    if (task->state != process_t::State::EXITED) {
        PANIC("only EXITED processes can go on exited list");
    }
    LOG_DEBUG("process %u going on exited list", task->pid);
    gExitedProcesses().set(task);
}

bool ProcessManager::collectany(pid_t* pid, process_exit_status_t* status) {
    auto c0 = gCurrentProcess->children.begin();
    auto ce = gCurrentProcess->children.end();

    for (; c0 != ce; ++c0) {
        auto child = *c0;
        if (child->state == process_t::State::EXITED) {
            *status = collect(*pid = child->pid);
            return true;
        }
    }

    // no child has EXITED yet
    return false;
}

process_exit_status_t ProcessManager::collect(pid_t pid) {
begin:
    auto task = getprocess(pid);

    LOG_DEBUG("process %u (%p) trying to collect process %u (%p)", gCurrentProcess->pid, gCurrentProcess, pid, task);

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
        deschedule(gCurrentProcess, process_t::State::COLLECTING);
        yield();
        goto begin;
    }

    LOG_DEBUG("task %u collecting dead task %u", gCurrentProcess->pid, task->pid);

    bool found = false;
    auto c0 = parent->children.begin();
    auto ce = parent->children.end();
    for (; c0 != ce; ++c0) {
        LOG_DEBUG("scanning children of parent %u, found %p %u", gCurrentProcess->pid, *c0, (*c0)->pid);
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

process_t* ProcessManager::cloneProcess(uintptr_t eip) {
    auto&& vm(VirtualPageManager::get());

    spawninfo_t si {
        cr3 : vm.cloneAddressSpace(),
        eip : (uintptr_t)clone_start,
        priority : gCurrentProcess->priority.prio,
        argument : eip,
        name : gCurrentProcess->path,
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
    bool ok0 = process->fds.set({nullptr, process->ttyinfo.ttyfile}, ttyfd0);
    bool ok1 = process->fds.set({nullptr, process->ttyinfo.ttyfile}, ttyfd1);
    bool ok2 = process->fds.set({nullptr, process->ttyinfo.ttyfile}, ttyfd2);
    if ((ok0 && 0 == ttyfd0) && (ok1 && 1 == ttyfd1) && (ok2 && 2 == ttyfd2)) {
        LOG_DEBUG("TTY setup complete - tty is %p ttyfile is %p", process->ttyinfo.tty, process->ttyinfo.ttyfile);
    } else {
        PANIC("unable to forward TTY to new process");
    }
}
