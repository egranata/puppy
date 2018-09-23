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
    class UptimeFile : public MemFS::File {
        public:
            UptimeFile() : MemFS::File("uptime") {}
            delete_ptr<MemFS::FileBuffer> content() override {
                string buf('\0', 24);
                sprint(&buf[0], 24, "%llu", TimeManager::get().millisUptime());
                return new MemFS::StringBuffer(buf);
            }
    };
    class NowFile : public MemFS::File {
        public:
            NowFile() : MemFS::File("now") {}
            delete_ptr<MemFS::FileBuffer> content() override {
                string buf('\0', 24);
                sprint(&buf[0], 24, "%llu", TimeManager::get().UNIXtime());
                return new MemFS::StringBuffer(buf);
            }
    };
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
        timer_dir->add(new UptimeFile());
        timer_dir->add(new NowFile());

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

uint64_t TimeManager::tick(InterruptStack& stack) {
    if (mTimeSource.millisPerTick == 0) {
        PANIC("TimeManager asked to tick without a known time source");
    }
    uint64_t new_count = __sync_add_and_fetch(&mMillisecondsSinceBoot, mTimeSource.millisPerTick);
    for (auto i = 0u; i < gMaxTickFunctions; ++i) {
        auto& ti = mTickHandlers.funcs[i];
        if (ti) {
            if (0 == --ti.every_countdown) {
                bool keep = ti.func(stack, new_count);
                if (!keep) {
                    unregisterTickHandler(i);
                } else {
                    ti.every_countdown = ti.every_N;
                }
            }
        }
    }

    return new_count;
}

uint64_t TimeManager::millisUptime() {
    return __sync_add_and_fetch(&mMillisecondsSinceBoot, 0);
}

TimeManager::TimeManager() : mMillisecondsSinceBoot(0), mUNIXTimestamp(0) {
    bzero(&mTickHandlers, sizeof(mTickHandlers));
    bzero(&mTimeSource, sizeof(mTimeSource));
}

size_t TimeManager::registerTickHandler(TimeManager::tick_func_t f, uint32_t every) {
    for (auto i = 0u; i < gMaxTickFunctions; ++i) {
        if (mTickHandlers.funcs[i]) continue;

        mTickHandlers.funcs[i].every_N = mTickHandlers.funcs[i].every_countdown = every;
        mTickHandlers.funcs[i].func = f;
        return i;
    }

    return -1;
}

void TimeManager::unregisterTickHandler(size_t i) {
    if (i == (size_t)-1) return;
    if (i >= gMaxTickFunctions) return;
    mTickHandlers.funcs[i].func = nullptr;
}

uint64_t TimeManager::UNIXtime() {
    return __sync_add_and_fetch(&mUNIXTimestamp, 0);
}

void TimeManager::UNIXtimeIncrement(uint64_t seconds) {
    __sync_add_and_fetch(&mUNIXTimestamp, seconds);
}
