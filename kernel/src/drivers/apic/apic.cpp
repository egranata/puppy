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

#include <kernel/drivers/apic/apic.h>
#include <kernel/log/log.h>
#include <kernel/i386/cpuid.h>
#include <kernel/i386/primitives.h>
#include <kernel/mm/virt.h>
#include <kernel/i386/idt.h>
#include <kernel/panic/panic.h>
#include <kernel/boot/phase.h>
#include <kernel/time/manager.h>
#include <kernel/drivers/pit/pit.h>

static void timer(GPR&, InterruptStack& stack, void*) {
    // LOG_DEBUG("APIC timer ticks");
    APIC::get().EOI();

    TimeManager::get().tick(stack);
}

namespace boot::apic {
        // TODO: should this be a configuration parameter?
        static constexpr uint32_t gMillisPerTick = 1;

        uint32_t init() {
            auto& apic(APIC::get());
            bootphase_t::printf("Calbrating APIC timer...");
            auto ticks_per_ms = apic.calibrate();
            bootphase_t::printf("%u ticks/ms\n", ticks_per_ms);
            Interrupts::get().sethandler(APIC::gAPICTimerIRQ, "APIC", timer);
            PIT::get().disable();
            TimeManager::get().registerTimeSource("APIC", gMillisPerTick);
            apic.tickEvery(gMillisPerTick);
            return 0;
        }

        bool fail(uint32_t) {
            return false;
        }
}

APIC& APIC::get() {
    static APIC gAPIC;

    return gAPIC;
}

void APIC::EOI() {
    auto reg = &mAPICRegisters[gEOIRegister];
    *reg = 0;
}

uint32_t APIC::getTimerCurrent() const {
    return mAPICRegisters[gTimerCurrentCount];
}

void APIC::tickEvery(uint32_t ms) {
    configure(mTicksPerMs * ms);
}

void APIC::configure(uint32_t ticks) {
    auto reg = &mAPICRegisters[gTimerDivideRegister];
    *reg = 0b111; // divide by 1
    LOG_DEBUG("timer divide register is %p - value is %p", reg, *reg);

    reg = &mAPICRegisters[gTimerRegister];
    *reg = 0x20000 | gAPICTimerIRQ; // deliver IRQ periodically
    LOG_DEBUG("timer register is %p - value is %p", reg, *reg);

    reg = &mAPICRegisters[gTimerInitialCount];
    LOG_DEBUG("initial count register is %p - value is %p", reg, *reg);
    *reg = ticks;
    LOG_DEBUG("initial count register is %p - value is %p", reg, *reg);

    // assume APIC is counting down and sending interrupts from here...
}

APIC::APIC() : mAPICRegisters(nullptr), mTicksPerMs(0) {
    if (CPUID::get().getFeatures().apic == false) {
        PANIC("system does not support APIC");
    } else {
        LOG_DEBUG("APIC bit set in CPUID info");
    }

    auto ia32apicbase = readmsr(gIA32_APIC_BASE);
    auto apicbase = (uint32_t)ia32apicbase & 0xFFFFF000;
    const bool bootCPU = ia32apicbase & (1 << 8);
    const bool apicON = ia32apicbase & (1 << 11);
    LOG_DEBUG("IA32_APIC_BASE = %llx - base address is %p, boot CPU is %u, apicON is %u", ia32apicbase, apicbase, bootCPU, apicON);
    if (apicON == false) {
        ia32apicbase |= (1 << 11);
        writemsr(gIA32_APIC_BASE, ia32apicbase);
        LOG_DEBUG("enabled APIC");
    }

    auto& vmm(VirtualPageManager::get());
    mAPICRegisters = (uint32_t*)vmm.getScratchPage(apicbase, VirtualPageManager::map_options_t::kernel().cached(false)).reset();
    LOG_DEBUG("APIC registers physically at %p, logically mapped at %p", apicbase, mAPICRegisters);

    auto reg = &mAPICRegisters[gSpuriousInterruptRegister];
    *reg = 0x1FF;
    LOG_DEBUG("spurious interrupt register is %p - value is %p", reg, *reg);
}
