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

#include <drivers/apic/apic.h>
#include <drivers/pit/pit.h>
#include <log/log.h>
#include <i386/idt.h>

static constexpr uint8_t gWantedCycles = 10;
static_assert(gWantedCycles > 2, "at least 3 cycles required for calibration");

static volatile uint8_t gNumCycles = 0;
static uint32_t gCyclesDelta[gWantedCycles + 1] = {0};
static uint32_t gTime = 0;

static bool pit_timer_helper(uint64_t) {
    auto apic_now = APIC::get().getTimerCurrent();

    if (gTime == 0) {
        gTime = apic_now;
    } else {
        gCyclesDelta[gNumCycles] = (gTime - apic_now);
        ++gNumCycles;
        gTime = apic_now;
    }

    return (gNumCycles != gWantedCycles);
}

uint32_t APIC::calibrate() {
    if (mTicksPerMs != 0) return mTicksPerMs;

    LOG_DEBUG("trying to calibrate APIC timer");

    configure(-1);

    PIT::get().addInterruptFunction(pit_timer_helper);
    Interrupts::get().enable();
    while(gNumCycles != gWantedCycles) __asm__ ( "hlt" );
    PIT::get().removeInterruptFunction(pit_timer_helper);

    configure(0);

    uint32_t cycles_per_10ms_avg = 0;
    uint32_t max_cycles = 0;
    uint32_t min_cycles = 0xFFFFFFFF;
    for (auto i = 0u; i < gWantedCycles; ++i) {
        auto cycles = gCyclesDelta[i];
        LOG_DEBUG("cycles value %u", cycles);
        if (cycles > max_cycles) max_cycles = cycles;
        if (cycles < min_cycles) min_cycles = cycles;
        cycles_per_10ms_avg += cycles;
    }

    LOG_DEBUG("added up %u cycle counts - removing max value %u and min value %u", cycles_per_10ms_avg, max_cycles, min_cycles);

    cycles_per_10ms_avg -= max_cycles;
    cycles_per_10ms_avg -= min_cycles;
    cycles_per_10ms_avg = cycles_per_10ms_avg / (gWantedCycles - 2);

    LOG_DEBUG("cycles per 10ms = %u, per ms will be 1/10th that", cycles_per_10ms_avg);

    return (mTicksPerMs = cycles_per_10ms_avg / 10);
}
