/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <kernel/synch/waitqueue.h>
#include <kernel/process/manager.h>
#include <kernel/process/process.h>

void WaitQueue::wait(process_t* task) {
    auto&& pm(ProcessManager::get());

    mProcesses.push(task);
    pm.deschedule(task, process_t::State::WAITQUEUE);
    pm.yield();
}

bool WaitQueue::wake(process_t* task) {
    auto&& pm(ProcessManager::get());

    if (task) {
        if (task->state == process_t::State::WAITQUEUE) {
            pm.ready(task);
            return true;
        }
    }

    return false;
}

process_t* WaitQueue::wakeone() {
    while (!mProcesses.empty()) {
        auto next = mProcesses.pop();
        if (wake(next)) return next;
    }

    return nullptr;
}

void WaitQueue::wakeall() {
    while (!mProcesses.empty()) {
        wake(mProcesses.pop());
    }
}

process_t* WaitQueue::peek() {
    if (mProcesses.empty()) return nullptr;

    return mProcesses.peek();
}

bool WaitQueue::empty() {
    return mProcesses.empty();
}
