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

#include <mutex.h>
#include <syscalls.h>

Mutex::Mutex(const char* key) {
    mHandle = mutexget_syscall((uint32_t)key) >> 1;
}

void Mutex::lock() {
    mutexlock_syscall(mHandle);
}

void Mutex::unlock() {
    mutexunlock_syscall(mHandle);
}

uintptr_t Mutex::handle() {
    return mHandle;
}
