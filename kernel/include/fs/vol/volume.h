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

#ifndef FS_VOL_VOLUME
#define FS_VOL_VOLUME

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>

class Volume : NOCOPY {
    protected:
        Volume();
    public:
        virtual ~Volume();

        bool read(uint32_t sector, uint16_t count, unsigned char* buffer);
        bool write(uint32_t sector, uint16_t count, unsigned char* buffer);

        virtual bool doRead(uint32_t sector, uint16_t count, unsigned char* buffer) = 0;
        virtual bool doWrite(uint32_t sector, uint16_t count, unsigned char* buffer) = 0;

        virtual uint8_t sysid() = 0;

        virtual size_t numsectors() const = 0;
        virtual size_t sectorsize() const { return 512; }
    private:
        void readAccounting(uint16_t sectors);
        void writeAccounting(uint16_t sectors);
};

#endif
