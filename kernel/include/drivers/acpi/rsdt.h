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

#ifndef DRIVERS_ACPI_RSDT
#define DRIVERS_ACPI_RSDT

#include <drivers/acpi/tablehdr.h>
#include <drivers/acpi/table.h>
#include <drivers/acpi/fadt.h>
#include <drivers/acpi/madt.h>

#include <sys/nocopy.h>

class RSDT : NOCOPY {
    public:
        size_t numtables() const;
        bool table(size_t, acpi_table_t**) const;
        acpi_fadt_table_t* fadt();
        acpi_madt_table_t* madt();
        const acpi_table_header_t& header();

    private:
        RSDT(uintptr_t);

        acpi_table_header_t mHeader;
        acpi_table_t *mTable;

        acpi_fadt_table_t* mFADT;
        acpi_madt_table_t *mMADT;
        friend class RSDP;
};

#endif
