/*
 * Copyright 2019 Google LLC
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

#ifndef I386_IOPORTS
#define I386_IOPORTS

#include <kernel/sys/nocopy.h>
#include <kernel/sys/stdint.h>

class IOPortsManager : NOCOPY {
    public:
        typedef uint16_t ioport_t;
        static constexpr size_t gNumPorts = 65536;

        // TODO: one could technically own an IOPort while the port it is associated to is freed
        class IOPort {
            public:
                void write8(uint8_t);
                void write16(uint16_t);
                void write32(uint32_t);

                uint8_t read8();
                uint16_t read16();
                uint32_t read32();
            private:
                IOPort(ioport_t port);
                ioport_t mPort;

                friend class IOPortsManager;
        };

        IOPortsManager();
        ~IOPortsManager();

        bool isPortFree(ioport_t);
        bool allocatePort(ioport_t);
        bool freePort(ioport_t);

        IOPort* getPort(ioport_t);
    private:
        static constexpr bool gFree = true;
        static constexpr bool gAllocated = false;
        bool mPortAllocation[gNumPorts];
};

#endif
