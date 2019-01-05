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

process_t::process_t() : tss(), args(nullptr), environ(nullptr), mmap(this), ttyinfo(), exitstatus(0), children() {
    pid = ppid = 0;
    state = State::NEW;
    sleeptill = 0;
    priority = {usedticks = 0,0};
    args = nullptr;
    path = nullptr;
    cwd = strdup("/");
    flags.flags = 0;

    // zero-out stats
    bzero(&this->memstats, sizeof(this->memstats));
    bzero(&this->iostats, sizeof(this->iostats));
    bzero(&this->runtimestats, sizeof(this->runtimestats));
    bzero(&this->priority, sizeof(this->priority));

    wakeReason.clear();
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
    copyArguments((const char**)other->args, false);
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

void process_t::copyArguments(const char** srcArgs, bool free_old_args) {
    if (free_old_args && args) {
        size_t i = 0;
        for(; args[i]; ++i) {
            free(args[i]);
        }
        free(args);
    }

    size_t sz = 0;
    while(srcArgs && srcArgs[sz]) ++sz;

    args = (char**)calloc(sz + 1, sizeof(const char*));
    for (size_t i = 0; i < sz; ++i) {
        args[i] = (char*)calloc(strlen(srcArgs[i]) + 1, sizeof(char));
        strcpy(args[i], srcArgs[i]);
    }
}
