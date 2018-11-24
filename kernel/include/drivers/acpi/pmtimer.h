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

#ifndef DRIVERS_ACPI_PMTIMER
#define DRIVERS_ACPI_PMTIMER

#include <kernel/sys/stdint.h>
#include <kernel/drivers/acpi/acpica/acpica.h>

class PMTimer {
    public:
        static PMTimer& get();

        uint32_t read();

        bool isThirtyTwoBits() const;
        uint16_t port() const;
    private:
        PMTimer();

        uint32_t doRead();

        uint16_t mPort;
        uint32_t mSize;
        uint32_t mZeroValue;
};

#endif
