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

#include <process/process.h>

process_t::process_t() : tss(), mmap(this), ttyinfo(), children() {
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
