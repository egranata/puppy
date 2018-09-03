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

#ifndef APIC_APIC
#define APIC_APIC

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>

class APIC : NOCOPY {
    public:
        static constexpr uint8_t gAPICTimerIRQ = 0xA0;
        static constexpr uint32_t gIA32_APIC_BASE = 0x1B;

        void EOI();

        static APIC& get();

        // this method attempts to use the PIT to figure out how many APIC timer ticks correspond to 1ms of real runtime
        // it should be called during kernel initialization only
        uint32_t calibrate();

        void tickEvery(uint32_t ms);

        uint32_t getTimerCurrent() const;
    private:
        void configure(uint32_t ticks);

        // Intel docs give offsets in bytes - but we use uint32_t* to enable proper access size, so divide accordingly
        static constexpr uint32_t gSpuriousInterruptRegister = 0xF0 / sizeof(uint32_t);
        static constexpr uint32_t gTimerRegister = 0x320 / sizeof(uint32_t);
        static constexpr uint32_t gTimerDivideRegister = 0x3E0 / sizeof(uint32_t);
        static constexpr uint32_t gTimerInitialCount = 0x380 / sizeof(uint32_t);
        static constexpr uint32_t gTimerCurrentCount = 0x390 / sizeof(uint32_t);
        static constexpr uint32_t gEOIRegister = 0xB0 / sizeof(uint32_t);

        uint32_t *mAPICRegisters;
        uint32_t mTicksPerMs;

        APIC();
};

#endif
