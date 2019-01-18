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

#include <kernel/drivers/ps2/controller.h>
#include <kernel/i386/primitives.h>
#include <kernel/i386/ioports.h>
#include <kernel/drivers/ps2/keyboard.h>
#include <kernel/log/log.h>
#include <kernel/boot/phase.h>
#include <kernel/drivers/pic/pic.h>
#include <kernel/drivers/acpi/match.h>

PS2Controller::Device::Device(uint8_t) {}

PS2Controller::StatusRegister::StatusRegister(uint8_t value) : mValue(value) {}

PS2Controller::Configuration::Configuration(uint8_t value) : mValue(value) {}

#define IS_BIT_SET(n) (0 != (mValue & (1 << n)))
#define IS_BIT_CLEAR(n) (0 == (mValue & (1 << n)))

bool PS2Controller::StatusRegister::outputBufferFull() const {
    return IS_BIT_SET(0);
}
bool PS2Controller::StatusRegister::inputBufferFull() const {
    return IS_BIT_SET(1);
}
bool PS2Controller::StatusRegister::systemFlag() const {
    return IS_BIT_SET(2);
}
bool PS2Controller::StatusRegister::isInputCommand() const {
    return IS_BIT_SET(3);
}
bool PS2Controller::StatusRegister::hasTimeoutError() const {
    return IS_BIT_SET(6);
}
bool PS2Controller::StatusRegister::hasParityError() const {
    return IS_BIT_SET(7);
}

bool PS2Controller::Configuration::firstPortIRQ() const {
    return IS_BIT_SET(0);
}
bool PS2Controller::Configuration::secondPortIRQ() const {
    return IS_BIT_SET(1);
}
bool PS2Controller::Configuration::systemFlag() const {
    return IS_BIT_SET(2);
}
bool PS2Controller::Configuration::firstPortClock() const {
    return IS_BIT_CLEAR(4);
}
bool PS2Controller::Configuration::secondPortClock() const {
    return IS_BIT_CLEAR(5);
}
bool PS2Controller::Configuration::firstPortTranslation() const {
    return IS_BIT_SET(6);
}

#undef IS_BIT_SET
#undef IS_BIT_CLEAR

#define DO_SET_BIT(n, value) if(value) { \
    mValue |= (1 << n); \
} else { \
    mValue &= ~(1 << n); \
}

void PS2Controller::Configuration::firstPortIRQ(bool value) {
    DO_SET_BIT(0, value);
}
void PS2Controller::Configuration::secondPortIRQ(bool value) {
    DO_SET_BIT(1, value);
}

void PS2Controller::Configuration::firstPortClock(bool enabled) {
    DO_SET_BIT(4, !enabled);
}
void PS2Controller::Configuration::secondPortClock(bool enabled) {
    DO_SET_BIT(5, !enabled);
}

void PS2Controller::Configuration::firstPortTranslation(bool value) {
    DO_SET_BIT(6, value);
}

#undef DO_SET_BIT

PS2Controller& PS2Controller::get() {
    static PS2Controller gController;

    return gController;
}

PS2Controller::StatusRegister PS2Controller::status() {
    return StatusRegister{inb(gStatusPort)};
}

PS2Controller::Configuration PS2Controller::configuration() {
    command(0x20); iowait();
    return Configuration{response()};
}
PS2Controller::Configuration PS2Controller::configuration(const PS2Controller::Configuration& config) {
    command(0x60, config.mValue);
    return configuration();
}

void PS2Controller::command(uint8_t byte) {
    while(status().inputBufferFull());
    outb(gCommandPort, byte);
    iowait();
}

void PS2Controller::command(uint8_t byte1, uint8_t byte2) {
    command(byte1);
    while(status().inputBufferFull());
    outb(gDataPort, byte2);
    iowait();
}

uint8_t PS2Controller::response() {
    while(true) {
        auto st = status();
        if (st.outputBufferFull()) return inb(gDataPort);
        iowait();
    }
}

uint8_t PS2Controller::response(uint64_t timeout, uint8_t def) {
    auto end = readtsc() + timeout;
    while(readtsc() < end) {
        auto st = status();
        if (st.outputBufferFull()) return inb(gDataPort);
        iowait();
    }
    return def;
}

void PS2Controller::disable(bool port1, bool port2) {
    if (port1) {
        command(0xAD);
    }
    if (port2) {
        command(0xA7);
    }
}

void PS2Controller::device1(uint8_t byte) {
    while(status().inputBufferFull());
    outb(gDataPort, byte);
}
void PS2Controller::device2(uint8_t byte) {
    while(status().inputBufferFull());
    outb(gCommandPort, 0xD4); iowait();
    while(status().inputBufferFull());
    outb(gDataPort, byte); iowait();
}

static constexpr uint8_t gControllerSelfTest = 0xAA;
static constexpr uint8_t gPort1SelfTest = 0xAB;
static constexpr uint8_t gEnablePort1 = 0xAE;

static constexpr uint8_t gDeviceSelfTest = 0xFF;
static constexpr uint8_t gDeviceNoScan = 0xF5;
static constexpr uint8_t gDeviceIdentify = 0xF2;
static constexpr uint8_t gDeviceScan = 0xF4;

PS2Controller::PS2Controller() : mDevice1(nullptr) {
    IOPortsManager::get().allocatePort(gDataPort);
    IOPortsManager::get().allocatePort(gStatusPort);

    inb(gDataPort);
    inb(gDataPort);
    inb(gDataPort);
    inb(gDataPort);
    LOG_DEBUG("buffer empty");
    auto configbyte = configuration();
    configbyte.firstPortIRQ(false);
    configbyte.firstPortClock(false);
    configuration(configbyte);

    device1(gDeviceNoScan);
    auto d1resp = response();
    if (0xFA != d1resp) {
        LOG_ERROR("device responded 0x%x, which is not 0xFA; out of here", d1resp);
        return;
    }

    auto kind1 = 0, kind2 = 0;
    device1(gDeviceIdentify);
    d1resp = response();
    if (0xFA != d1resp) {
        LOG_ERROR("device responded 0x%x, which is not 0xFA; out of here", d1resp);
        return;
    }
    kind1 = response(50000000, 0xFF);
    kind2 = response(50000000, 0xAA);
    LOG_INFO("PS/2 device1 identified as 0x%x 0x%x", kind1, kind2);

    if (kind1 != 0xAB || (kind2 != 0x83 && kind2 != 0x41 && kind2 != 0xC1)) {
        LOG_DEBUG("no keyboard detected. don't know how to try port2 yet.");
        return;
    }  

    device1(gDeviceScan);
    d1resp = response();
    if (0xFA != d1resp) {
        LOG_ERROR("device responded 0x%x, which is not 0xFA; out of here", d1resp);
        return;
    }

    configbyte.firstPortIRQ(true);
    configbyte.firstPortClock(true);
    configbyte.firstPortTranslation(false);
    configuration(configbyte);
    mDevice1 = new PS2Keyboard(1);

    LOG_DEBUG("PS/2 configuration complete");
}

PS2Controller::Device* PS2Controller::getDevice1() {
    return mDevice1;
}
void PS2Controller::Device::send(uint8_t devid, uint8_t byte) {
    devid == 1 ? PS2Controller::device1(byte) : PS2Controller::device2(byte);
}
uint8_t PS2Controller::Device::receive(uint8_t) {
    return PS2Controller::response();
}

static bool ps2_acpi(const AcpiDeviceManager::acpica_device_t&) {
        auto&& ps2(PS2Controller::get());
        auto device = ps2.getDevice1();
        if (device == nullptr) {
            LOG_ERROR("could not find PS/2 keyboard.");
            return false;
        }
        if (device->getType() != PS2Controller::Device::Type::KEYBOARD) {
            LOG_ERROR("could not find PS/2 keyboard.");
            return false;
        }

        PIC::get().accept(1);
        return true;
}
ACPI_HID_MATCH(PNP0303, ps2_acpi);
