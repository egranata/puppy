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

process_t::process_t() : tss(), mmap(this), ttyinfo(), exitstatus(0), children() {
    pid = ppid = 0;
    state = State::AVAILABLE;
    sleeptill = 0;
    priority = {usedticks = 0,0};
    args = nullptr;
    path = nullptr;
}

process_t::ttyinfo_t::ttyinfo_t() : tty(nullptr), ttyfile(nullptr) {
}

process_t::ttyinfo_t::ttyinfo_t(TTY* t) {
    tty = t;
    ttyfile = new TTYFile(tty);
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

    other->state = process_t::State::NEW;
    other->sleeptill = 0;
    other->priority = priority;
    other->usedticks = 0;

    other->mmap.clone(&mmap);
    // tty is cloned in ProcessManager

    other->flags.system = flags.system;

    other->memstats.allocated = 0;
    other->memstats.pagefaults = 0;

    other->runtimestats.runbegin = 0;
    other->runtimestats.runtime = 0;
}
