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

#include <kernel/time/manager.h>
#include <kernel/libc/string.h>
#include <kernel/log/log.h>
#include <kernel/panic/panic.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/libc/sprint.h>

namespace {
    class TimeFile : public MemFS::File {
        protected:
            TimeFile(const char* name) : MemFS::File(name) {}
            virtual uint64_t value() = 0;
        public:
            delete_ptr<MemFS::FileBuffer> content() override {
                string buf('\0', 24);
                sprint(&buf[0], 24, "%llu", value());
                return new MemFS::StringBuffer(buf);
            }
    };

    #define FILE(name, expression) class name ## file : public TimeFile { \
        public: \
            name ## file() : TimeFile(#name) {} \
        protected: \
            uint64_t value() override { return expression ; } \
    }

    FILE(uptime, TimeManager::get().millisUptime());
    FILE(now, TimeManager::get().UNIXtime());
    FILE(boot_duration, TimeManager::get().millisBootTime());

    #undef FILE
}

namespace boot::time {
    uint32_t init() {
        TimeManager::get();
        return 0;
    }
}

namespace boot::time_files {
    uint32_t init() {
        auto timer_dir = DevFS::get().getDeviceDirectory("time");
        timer_dir->add(new uptimefile());
        timer_dir->add(new nowfile());
        timer_dir->add(new boot_durationfile());
        return 0;
    }
}

TimeManager& TimeManager::get() {
    static TimeManager gManager;

    return gManager;
}

void TimeManager::registerTimeSource(const char* name, uint32_t ms) {
    mTimeSource.name = name;
    mTimeSource.millisPerTick = ms;
}

uint64_t TimeManager::millisPerTick() {
    return mTimeSource.millisPerTick;
}

time_tick_callback_t::yield_vote_t TimeManager::tick(InterruptStack& stack) {
    time_tick_callback_t::yield_vote_t want_yield = time_tick_callback_t::no_yield;

    if (mTimeSource.millisPerTick == 0) {
        PANIC("TimeManager asked to tick without a known time source");
    }
    uint64_t new_count = __sync_add_and_fetch(&mMillisecondsSinceBoot, mTimeSource.millisPerTick);
    for (auto i = 0u; i < gMaxTickFunctions; ++i) {
        auto& ti = mTickHandlers.funcs[i];
        if (ti) {
            if (0 == ti.every_countdown) {
                if (time_tick_callback_t::yield == ti.callback.run(stack, new_count)) {
                    want_yield = time_tick_callback_t::yield;
                }
                ti.every_countdown = ti.every_N;
            } else {
                --ti.every_countdown;
            }
        }
    }

    return want_yield;
}

uint64_t TimeManager::millisUptime() {
    return __sync_add_and_fetch(&mMillisecondsSinceBoot, 0);
}

TimeManager::TimeManager() : mMillisecondsSinceBoot(0), mBootDurationMillis(0), mUNIXTimestamp(0) {
    bzero(&mTickHandlers, sizeof(mTickHandlers));
    bzero(&mTimeSource, sizeof(mTimeSource));
}

size_t TimeManager::registerTickHandler(time_tick_callback_t::func_f f, void* baton, uint32_t every_N) {
    for (auto i = 0u; i < gMaxTickFunctions; ++i) {
        auto& th(mTickHandlers.funcs[i]);

        if (th) continue;
        else {
            th.every_N = th.every_countdown = every_N;
            th.callback.set(f, baton);
            return i;
        }
    }

    return -1;
}

void TimeManager::unregisterTickHandler(size_t i) {
    if (i == (size_t)-1) return;
    if (i >= gMaxTickFunctions) return;
    mTickHandlers.funcs[i].callback.clear();
}

uint64_t TimeManager::UNIXtime() {
    return __sync_add_and_fetch(&mUNIXTimestamp, 0);
}

void TimeManager::UNIXtimeIncrement(uint64_t seconds) {
    __sync_add_and_fetch(&mUNIXTimestamp, seconds);
}

void TimeManager::bootCompleted() {
    mBootDurationMillis = mMillisecondsSinceBoot;
}

uint64_t TimeManager::millisBootTime() const {
    return mBootDurationMillis;
}
