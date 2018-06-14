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

#ifndef DRIVERS_ACPI_TABLEHDR
#define DRIVERS_ACPI_TABLEHDR

#include <kernel/sys/stdint.h>

struct acpi_table_header_t {
    char sig[4];
    uint32_t len;
    uint8_t rev;
    uint8_t checksum;
    char oem[6];
    char oemtableid[8];
    uint32_t oemrev;
    uint32_t creatorid;
    uint32_t creatorrev;
} __attribute((packed));


#endif