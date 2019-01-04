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

#ifndef TIME_CALLBACK
#define TIME_CALLBACK

#include <kernel/i386/idt.h>

struct time_tick_callback_t {
    using func_f = void(*)(InterruptStack&, uint64_t, void*);
    func_f func;
    void* baton;

    time_tick_callback_t(func_f = nullptr, void* = nullptr);

    void set(func_f, void*);
    void clear();

    explicit operator bool() const;
    void run(InterruptStack&, uint64_t);
};

#endif
