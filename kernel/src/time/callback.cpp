/*
 * Copyright 2019 Google LLC
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

#include <kernel/time/callback.h>
#include <kernel/log/log.h>

time_tick_callback_t::time_tick_callback_t(func_f f, void* b) {
    set(f,b);
}

void time_tick_callback_t::set(func_f f, void* b) {
    func = f;
    baton = b;
}

void time_tick_callback_t::clear() {
    set(nullptr, nullptr);
}

time_tick_callback_t::operator bool() const {
    return func != nullptr;
}

time_tick_callback_t::yield_vote_t time_tick_callback_t::run(InterruptStack& is, uint64_t count) {
    yield_vote_t vote = no_yield;
    if(func) vote = func(is, count, baton);
    return vote;
}
