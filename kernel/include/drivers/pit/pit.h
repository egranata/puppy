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

#ifndef PIT_PIT
#define PIT_PIT

#include <kernel/sys/stdint.h>

class PIT {
    public:
        // return false to remove this function from the list
        typedef bool(*pit_func_f)(uint64_t);
        static constexpr uint8_t gIRQNumber = 0;

        static constexpr uint8_t gDataPort = 0x40;
        static constexpr uint8_t gCommandPort = 0x43;

        static constexpr uint32_t gTickDuration = 10;

        static PIT& get();

        static uint64_t getBootTimestamp();

        // in order to keep things expedient there is only a small number
        // of allowed slots for interrupt functions - these functions also run
        // as part of the PIT interrupt handler, so the goal is for them to be
        // as quick as possible (e.g. no logging!)
        bool addInterruptFunction(pit_func_f);
        void removeInterruptFunction(pit_func_f);

        void disable();
    private:
        PIT();

        PIT(const PIT&) = delete;
        PIT(PIT&&) = delete;
        PIT& operator=(const PIT&) = delete;
};

#endif
