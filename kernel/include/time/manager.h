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

#ifndef TIME_MANAGER
#define TIME_MANAGER

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>
#include <kernel/libc/function.h>
#include <kernel/i386/idt.h>
#include <kernel/time/callback.h>

class TimeManager : NOCOPY {
    public:
        static TimeManager& get();

        void registerTimeSource(const char*, uint32_t);

        uint64_t tick(InterruptStack&);

        uint64_t millisUptime();

        uint64_t millisPerTick();

        uint64_t UNIXtime();
        void UNIXtimeIncrement(uint64_t seconds = 1);

        size_t registerTickHandler(time_tick_callback_t::func_f, void*, uint32_t every_N);
        void unregisterTickHandler(size_t);

        void bootCompleted();

        uint64_t millisBootTime() const;
    private:
        static constexpr size_t gMaxTickFunctions = 10;

        uint64_t mMillisecondsSinceBoot;

        uint64_t mBootDurationMillis;

        uint64_t mUNIXTimestamp;

        struct {
            struct {
                time_tick_callback_t callback;
                uint32_t every_N; // only run func every N ticks
                uint32_t every_countdown; // how many ticks left before the next execution of func

                explicit operator bool() const { return callback.operator bool(); }
            } funcs[gMaxTickFunctions];
        } mTickHandlers;

        struct {
            const char* name;
            uint32_t millisPerTick;
        } mTimeSource;

        TimeManager();
};

#endif
