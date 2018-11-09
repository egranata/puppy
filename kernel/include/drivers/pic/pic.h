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

#ifndef PIC_PIC
#define PIC_PIC

#include <kernel/sys/stdint.h>
#include <kernel/drivers/acpi/acpica/acpica.h>

class PIC {
    public:
        static constexpr uint8_t gEndOfInterrupt = 0x20;
        static uint8_t gOffset(uint8_t pic);
        static uint8_t gIRQNumber(uint8_t irq);

        static PIC& get();

        ACPI_STATUS setupACPI();

        static void eoi(uint8_t irq);

        uint8_t irr(uint8_t pic);
        uint8_t isr(uint8_t pic);

        PIC& ignore(uint8_t irq);
        PIC& accept(uint8_t irq);

    private:
        static uint16_t commandPort(uint8_t pic);
        static uint16_t dataPort(uint8_t pic);

        PIC();

        PIC(const PIC&) = delete;
        PIC(PIC&&) = delete;
        PIC& operator=(const PIC&) = delete;
};

#endif
