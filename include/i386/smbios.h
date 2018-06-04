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

#ifndef I386_SMBIOS
#define I386_SMBIOS

#include <sys/stdint.h>
#include <sys/nocopy.h>

class SMBIOS : NOCOPY {
    public:
        static SMBIOS* get();
    
        const char* getBIOSVendor();
        const char* getBIOSVersion();

        const char* getSystemManufacturer();
        const char* getSystemProductName();
        const char* getSystemSerial();

    private:
        struct smbios_entrypoint_t {
            uint32_t anchor;
            uint8_t checksum;
            uint8_t size;
            uint8_t major;
            uint8_t minor;
            uint16_t maxsize;
            uint8_t revision;
            uint8_t reserved[5];
            uint8_t dmi[5];
            uint8_t ichecksum;
            uint16_t tbllen;
            uint32_t tblptr;
            uint8_t numtbls;
            uint8_t bcdrev;
        } __attribute__((packed));

        struct smbios_biosinfo_t {
            char* vendor;
            char* version;
        };

        struct smbios_systeminfo_t {
            char* manufacturer;
            char* name;
            char* serial;
        };

        struct smbios_cpu_info_t {
            char* socket;
            uint8_t proctype;
            uint8_t procfamily;
            uint64_t procid;
            char* vers;
            uint8_t voltage;
            uint16_t extclock;
            uint16_t maxspeed;
            uint16_t curspeed;
            uint8_t status;
            uint8_t upgrade;
        };

        struct smbios_table_header_t {
            uint8_t type;
            uint8_t len;
            uint16_t handle;

            uint8_t byte(uint8_t idx);
            uint8_t *str();
            char *str(uint8_t idx);
            uint16_t word(uint8_t idx);
            uint64_t qword(uint8_t idx);
        } __attribute((packed));
        SMBIOS();
        bool valid() const;

        smbios_entrypoint_t mEntryTable;
        smbios_biosinfo_t mBiosInfo;
        smbios_systeminfo_t mSystemInfo;
        smbios_cpu_info_t mCPUInfo;
        bool mValid;
};

#endif
