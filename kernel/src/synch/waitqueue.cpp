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

#define LOG_LEVEL 2
#include <kernel/log/log.h>

void WaitQueue::pushToQueue(process_t* task) {
    auto&& pm(ProcessManager::get());

    mProcesses.push({task->waitToken, task});
    pm.deschedule(task, process_t::State::WAITING, this);
}

void WaitQueue::wait(process_t* task) {
    LOG_DEBUG("task %u entering wait queue 0x%p at token %llu", task->pid, this, task->waitToken);
    pushToQueue(task);
}

void WaitQueue::yield(process_t* task, uint32_t timeout) {
    auto&& pm(ProcessManager::get());

    wait(task);
    if (timeout) pm.sleep(timeout);
    else pm.yield();
}

bool WaitQueue::wake(const queue_entry_t& q) {
    auto&& pm(ProcessManager::get());

    if (q.process) {
        LOG_DEBUG("wait queue 0x%p trying to wake task %u (state == %u)", this, q.process->pid, (uint8_t)q.process->state);
        if (q.process->state == process_t::State::WAITING) {
            if (q.process->waitToken == q.token) {
                LOG_DEBUG("task wake happening");
                pm.ready(q.process, this);
                return true;
            } else {
                LOG_WARNING("wait queue's token %llu mismatches; ignoring wake", q.token);
                return false;
            }
        }
    }

    return false;
}

process_t* WaitQueue::wakeone() {
    while (!mProcesses.empty()) {
        auto next = mProcesses.pop();
        if (wake(next)) {
            LOG_DEBUG("wait queue 0x%p woke up the one next %u", this, next.process->pid);
            return next.process;
        }
    }

    return nullptr;
}

void WaitQueue::wakeall() {
    while (!mProcesses.empty()) {
        auto next = mProcesses.pop();
        if (wake(next)) {
            LOG_DEBUG("wait queue 0x%p woke up %u", this, next.process->pid);
        }
    }
}

process_t* WaitQueue::peek() {
    if (mProcesses.empty()) return nullptr;

    return mProcesses.peek().process;
}

bool WaitQueue::empty() {
    return mProcesses.empty();
}

void WaitQueue::remove(process_t* task) {
    mProcesses.remove(is_same_process, task);
}
