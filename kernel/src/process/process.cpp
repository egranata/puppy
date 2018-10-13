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

#include <kernel/process/process.h>
#include <kernel/libc/string.h>
#include <kernel/i386/primitives.h>

process_t::process_t() : tss(), environ(nullptr), mmap(this), ttyinfo(), exitstatus(0), children() {
    pid = ppid = 0;
    state = State::AVAILABLE;
    sleeptill = 0;
    priority = {usedticks = 0,0};
    args = nullptr;
    path = nullptr;
    cwd = strdup("/");
    flags.due_for_reschedule = false;

    memstats.allocated = memstats.available = 0;
    memstats.pagefaults = 0;

    iostats.read = iostats.written = 0;

    bzero(&this->priority, sizeof(this->priority));
}

process_t::ttyinfo_t::ttyinfo_t() : tty(nullptr), ttyfile(nullptr) {
}

process_t::ttyinfo_t::ttyinfo_t(TTY* t) : ttyfile(t) {
    tty = t;
}

process_t::ttyinfo_t& process_t::ttyinfo_t::operator=(const ttyinfo_t& info) {
    tty = info.tty;
    ttyfile.setTTY(tty);
    return *this;
}

// TODO: make tty and ttyfile be refcounted
process_t::ttyinfo_t::~ttyinfo_t() = default;

MemoryManager* process_t::getMemoryManager() {
    return &mmap;
}

void process_t::clone(process_t* other) {
    other->tss = tss;
    other->cr0 = cr0;

    other->path = strdup(path);
    other->args = strdup(args);
    other->cwd = strdup(cwd);

    other->environ = nullptr; // the environment does not need to be copied

    other->state = process_t::State::NEW;
    other->sleeptill = 0;
    other->priority = priority;
    other->usedticks = 0;

    mmap.clone(&other->mmap);
    // tty is cloned in ProcessManager

    fpsave((uintptr_t)&fpstate[0]);
    memcpy(other->fpstate, fpstate, sizeof(fpstate));

    other->flags.system = flags.system;

    other->memstats.allocated = memstats.allocated;
    other->memstats.allocated = 0;
    other->memstats.pagefaults = 0;

    other->iostats.read = other->iostats.written = 0;

    other->runtimestats.runtime = 0;

    other->priority = priority;
}
