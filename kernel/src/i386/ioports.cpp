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

#include <kernel/i386/ioports.h>
#include <kernel/i386/primitives.h>
#include <kernel/panic/panic.h>

#include <kernel/log/log.h>

IOPortsManager::IOPortsManager() {
    for(size_t i = 0; i < gNumPorts; ++i) {
        mPortAllocation[i] = gFree;
    }
}

IOPortsManager::~IOPortsManager() {
    for(size_t i = 0; i < gNumPorts; ++i) {
        if (gAllocated == mPortAllocation[i]) {
            LOG_ERROR("I/O port %u allocated while manager going away", i);
            PANIC("I/O port allocated but handler going down");
        }
    }
}

bool IOPortsManager::isPortFree(ioport_t port) {
    return gFree == mPortAllocation[port];
}

bool IOPortsManager::allocatePort(ioport_t port) {
    if (isPortFree(port)) {
        mPortAllocation[port] = gAllocated;
        return true;
    }
    return false;
}

bool IOPortsManager::freePort(ioport_t port) {
    if (isPortFree(port)) return false;
    mPortAllocation[port] = gFree;
    return true;
}

IOPortsManager::IOPort* IOPortsManager::getPort(ioport_t port) {
    if (isPortFree(port)) return nullptr;
    return new IOPort(port);
}

IOPortsManager::IOPort::IOPort(ioport_t port) : mPort(port) {}

void IOPortsManager::IOPort::write8(uint8_t value) {
    outb(mPort, value);
}
void IOPortsManager::IOPort::write16(uint16_t value) {
    outw(mPort, value);
}
void IOPortsManager::IOPort::write32(uint32_t value) {
    outl(mPort, value);
}

uint8_t IOPortsManager::IOPort::read8() {
    return inb(mPort);
}
uint16_t IOPortsManager::IOPort::read16() {
    return inw(mPort);
}
uint32_t IOPortsManager::IOPort::read32() {
    return inl(mPort);
}
