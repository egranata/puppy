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

#ifndef PROCESS_BITMAP
#define PROCESS_BITMAP

#include <kernel/sys/stdint.h>
#include <kernel/panic/panic.h>

template<size_t NumProcesses, size_t NumBytes = NumProcesses / 8, typename PidType = uint16_t>
class ProcessBitmap {
    public:
        ProcessBitmap() {
            bzero(&mBitmap[0], sizeof(mBitmap));
        }

        PidType next() {
            for (auto i = 0u; i < NumBytes; ++i) {
                auto& entry = mBitmap[i];
                if (0xFF == entry) continue;
                for (auto j = 0u; j < 8; ++j) {
                    if (0 == (entry & gBitmasks[j])) {
                        entry |= gBitmasks[j];
                        return 8*i + j;
                    }
                }
            }

            PANIC("out of process entries");
        }

        void free(PidType p) {
            auto B = (p / 8);
            auto b = (p % 8);
            mBitmap[B] = mBitmap[B] & ~gBitmasks[b];
        }

        void reserve(PidType p) {
            auto B = (p / 8);
            auto b = (p % 8);
            mBitmap[B] = mBitmap[B] | gBitmasks[b];            
        }
    private:
        static_assert(8 * NumBytes == NumProcesses, "process count must be a multiple of 8");

        static constexpr uint8_t gBitmasks[8] = {
            1 << 0,
            1 << 1,
            1 << 2,
            1 << 3,
            1 << 4,
            1 << 5,
            1 << 6,
            1 << 7
        };
        uint8_t mBitmap[NumBytes];
};

template<size_t A, size_t B, typename C>
constexpr decltype(ProcessBitmap<A,B,C>::gBitmasks) ProcessBitmap<A,B,C>::gBitmasks;

#endif
