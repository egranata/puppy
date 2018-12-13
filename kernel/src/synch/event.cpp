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

#include <kernel/synch/event.h>
#include <kernel/process/current.h>
#include <kernel/libc/string.h>

Event::Event(const char* name) {
    mKey = strdup(name);
    mRaised = false;
}

Event::~Event() {
    free((void*)mKey);
}

const char* Event::key() {
    return mKey;
}

void Event::raise() {
    mRaised = true;
    waitqueue()->wakeall();
}
void Event::lower() {
    mRaised = false;
}

bool Event::raised() const {
    return mRaised;
}

bool Event::wait(uint32_t timeout) {
    bool wait = true;
    while(true) {
        if (mRaised) {
            return true;
        } else {
            if (!wait) return false;
            waitqueue()->yield(gCurrentProcess, timeout);
            if (!myWake(gCurrentProcess)) return false;
            wait = (timeout > 0);
        }
    }
}
