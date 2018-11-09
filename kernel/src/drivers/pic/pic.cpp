// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <kernel/drivers/pic/pic.h>
#include <kernel/i386/primitives.h>
#include <kernel/panic/panic.h>
#include <kernel/log/log.h>

namespace boot::pic {
        uint32_t init() {
            PIC::get();
            return 0;
        }

        bool fail(uint32_t) {
            return false;
        }
}

PIC& PIC::get() {
    static PIC gPIC;

    return gPIC;
}

#define PIC_PANIC if (pic > 1) { PANIC("only PICs 0 and 1 exist"); }

uint16_t PIC::commandPort(uint8_t pic) {
    PIC_PANIC;
    return (pic == 0 ? 0x20 : 0xA0);
}
uint16_t PIC::dataPort(uint8_t pic) {
    PIC_PANIC;
    return (pic == 0 ? 0x21 : 0xA1);
}

uint8_t PIC::gOffset(uint8_t pic) {
    PIC_PANIC;
    return 0x40 + 8*pic;
}

uint8_t PIC::gIRQNumber(uint8_t irq) {
    return 0x40 + irq;
}

void PIC::eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(commandPort(1), gEndOfInterrupt);
    }
    outb(commandPort(0), gEndOfInterrupt);
}

ACPI_STATUS PIC::setupACPI() {
    ACPI_OBJECT_LIST argl;
    ACPI_OBJECT arg;

    arg.Type = ACPI_TYPE_INTEGER;
    arg.Integer.Value = 0; // 0 is PIC; 1 is APIC

    argl.Count = 1;
    argl.Pointer = &arg;

    ACPI_STATUS ok = AcpiEvaluateObject(ACPI_ROOT_OBJECT, (char*)"_PIC", &argl, nullptr);
    if (ok == AE_OK) return ok;
    if (ok == AE_NOT_FOUND) {
        TAG_WARNING(ACPICA, "_PIC not found in ACPI; ignoring");
        return AE_OK;
    } else {
        TAG_ERROR(ACPICA, "_PIC execution failed: %u", ok);
        return ok;
    }
}

PIC::PIC() {
    auto a0 = inb(dataPort(0));
    auto a1 = inb(dataPort(1));

    LOG_DEBUG("PIC data ports %u %u", a0, a1);

    outb(commandPort(0), 0x11); iowait();
    outb(commandPort(1), 0x11); iowait();
    outb(dataPort(0), gOffset(0)); iowait();
    outb(dataPort(1), gOffset(1)); iowait();
    outb(dataPort(0), 4); iowait();
    outb(dataPort(1), 2); iowait();
    outb(dataPort(0), 1); iowait();
    outb(dataPort(1), 1); iowait();

    outb(dataPort(0), a0); iowait();
    outb(dataPort(1), a1); iowait();

    LOG_INFO("PIC initialized");

    // set to ignore all IRQs..
    for (auto i = 0u; i < 16u; ++i) {
        ignore(i);
    }
    // except IRQ2 on the master - this is the IRQ that enables
    // cascading with the slave PIC and should always be enabled
    accept(2);
}

uint8_t PIC::irr(uint8_t pic) {
    PIC_PANIC;
    outb(commandPort(pic), 0x0A); iowait();
    return inb(commandPort(pic));
}

uint8_t PIC::isr(uint8_t pic) {
    PIC_PANIC;
    outb(commandPort(pic), 0x0B); iowait();
    return inb(commandPort(pic));
}

PIC& PIC::ignore(uint8_t irq) {
    uint8_t pic;
    if (irq < 8) {
        pic = 0;
    } else {
        pic = 1;
        irq -= 8;
    }
    auto data = inb(dataPort(pic));
    data |= (1 << irq);
    outb(dataPort(pic), data); iowait();

    return *this;
}

PIC& PIC::accept(uint8_t irq) {
    uint8_t pic;
    if (irq < 8) {
        pic = 0;
    } else {
        pic = 1;
        irq -= 8;
    }
    auto data = inb(dataPort(pic));
    data &= ~(1 << irq);
    outb(dataPort(pic), data); iowait();

    return *this;
}
