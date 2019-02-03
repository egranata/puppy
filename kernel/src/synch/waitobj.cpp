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

#include <kernel/synch/waitobj.h>
#include <kernel/process/process.h>

WaitableObject::WaitableObject(Kind k) : mKind(k), mWQ() {}

WaitableObject::Kind WaitableObject::getKind() const {
    return mKind;
}

WaitQueue* WaitableObject::waitqueue() {
    return &mWQ;
}

bool WaitableObject::myWake(process_t* task) {
    return (task->wakeReason.waitable == waitqueue());
}

WaitableObject::~WaitableObject() = default;

