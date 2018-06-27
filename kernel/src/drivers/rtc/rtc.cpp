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

#include <kernel/drivers/rtc/rtc.h>
#include <kernel/i386/primitives.h>
#include <kernel/panic/panic.h>
#include <kernel/i386/idt.h>
#include <kernel/drivers/pic/pic.h>

#include <kernel/log/log.h>

namespace boot::rtc {
    uint32_t init() {
        auto &&rtc(RTC::get());
    	LOG_INFO("RTC gathered - timestamp is %llu", rtc.timestamp());
        PIC::get().accept(RTC::gIRQNumber);
        return 0;
    }

    bool fail(uint32_t) {
        return false;
    }
}

bool RTC::cmos_now_t::time_t::operator==(const time_t& t) const {
    return (t.second == second) && (t.minute == minute) && (t.hour == hour);
}

bool RTC::cmos_now_t::date_t::operator==(const date_t& d) const {
    return (d.day == day) && (d.month == month) && (d.year == year);
}

bool RTC::cmos_now_t::operator==(const cmos_now_t& n) const {
    return (n.date == date) && (n.time == time);
}

static bool leap(uint16_t year) {
    if (0 != (year % 4)) return false;
    if (0 != (year % 100)) return true;
    return 0 == (year % 400);    
}

bool RTC::cmos_now_t::leap() const {
    return ::leap(date.year);
}

static uint8_t gDays[] = {
    0,
    31, // Jan
    28, // Feb
    31, // Mar
    30, // Apr
    31, // May
    30, // Jun
    31, // Jul
    31, // Aug
    30, // Sep
    31, // Oct
    30, // Nov
    31 // Dec
};

static uint8_t gLeapDays[] = {
    0,
    31, // Jan
    29, // Feb
    31, // Mar
    30, // Apr
    31, // May
    30, // Jun
    31, // Jul
    31, // Aug
    30, // Sep
    31, // Oct
    30, // Nov
    31 // Dec
};

static constexpr uint64_t gSecondsInDay = 60 /* secs/min */ * 60 /* min/hr */ * 24 /* hr */;

uint64_t RTC::cmos_now_t::timestamp() const {
    uint64_t val = 0;

    // add up all years until the current one
    for (auto y = 1970u; y < date.year; ++y) {
        val += gSecondsInDay * (::leap(y) ? 366 : 365);
    }

    // for the current year, add up months fully elapsed
    for (auto m = 1; m < date.month; ++m) {
        val += gSecondsInDay * m[leap() ? gLeapDays : gDays];
    }

    // for the current month, add up days fully elapsed
    val += (date.day - 1) * gSecondsInDay;

    // and now add time components
    val += time.second + (60 * (time.minute + (60 * time.hour)));

    return val;
}

static uint64_t gTimestamp = 0;
static uint8_t gIncrement = 0;

static void rtchandler(GPR&, InterruptStack&, void*) {
    outb(RTC::gSelectPort, RTC::gNMI | RTC::gStatusRegisterC);
    inb(RTC::gDataPort);

    __sync_fetch_and_add(&gTimestamp, gIncrement);
    gIncrement = 1 - gIncrement;
    PIC::eoi(8);
}

uint64_t RTC::timestamp() const {
    return __sync_fetch_and_add(&gTimestamp, 0);
}

RTC& RTC::get() {
    static RTC gRTC;
    
    return gRTC;
}

#define BCD_TO_BINARY(bcd) bcd = ((bcd & 0x0F) + ((bcd / 16) * 10))

RTC::RTC() {
    auto regb = read(gStatusRegisterB);
    bool militarytime = (0 != (regb & 2));
    bool binary = (0 != (regb & 0x4));

    LOG_DEBUG("system provides military time: %s, binary values: %s",
        militarytime ? "yes" : "no",
        binary ? "yes" : "no");

    bool again = true;
    cmos_now_t rightnow;
    while(again) {
        while(updating());
        auto now1 = now();
        while(updating());
        auto now2 = now();
        if (now1 == now2) {
            rightnow = now1;
            again = false;
        }
    }

    if (!binary) {
        BCD_TO_BINARY(rightnow.time.second);
        BCD_TO_BINARY(rightnow.time.minute);
        BCD_TO_BINARY(rightnow.time.hour);

        BCD_TO_BINARY(rightnow.date.day);
        BCD_TO_BINARY(rightnow.date.month);
        BCD_TO_BINARY(rightnow.date.year);

        if (!militarytime) {
            if (rightnow.time.hour == 12) {
                // midnight
                rightnow.time.hour = 0;
            } else if (rightnow.time.hour & 0x80) {
                // PM
                rightnow.time.hour = 12 + (rightnow.time.hour & 0x7F);
            }
        }

        if (rightnow.date.year >= 18) {
            rightnow.date.year += 2000;
        } else {
            // if the year is < 18, this is most likely 2100+
            // as development started in 2018.. and time travel
            // has not been invented yet
            rightnow.date.year += 2100;
        }
    }

    LOG_DEBUG("it is %d:%d:%d on %d/%d/%d",
        rightnow.time.hour, rightnow.time.minute, rightnow.time.second,
        rightnow.date.month, rightnow.date.day, rightnow.date.year);

    __sync_fetch_and_add(&gTimestamp, rightnow.timestamp());

    // interrupts should be disabled when doing this
    bool IF = (readflags() & 0x200) != 0;
    if (IF) PANIC("cannot configure RTC with IF == 1");

    auto irq = PIC::gIRQNumber(8);
    Interrupts::get().sethandler(irq, ::rtchandler);

    auto regA = read(gStatusRegisterA);
    LOG_DEBUG("RTC regA = %u", regA);
    regA |= 0xF; // rate == 15 --> 2Hz
    write(gStatusRegisterA, regA);
    auto regB = read(gStatusRegisterB);
    LOG_DEBUG("RTC regB = %u", regB);    
    regB |= 0x40; // turn on IRQ
    write(gStatusRegisterB, regB);
    auto regC = read(gStatusRegisterC);
    LOG_DEBUG("RTC regC = %u", regC);
}

RTC::cmos_now_t RTC::now() {
    cmos_now_t now {
        time : {
            hour : read(gHoursRegister),
            minute : read(gMinutesRegister),
            second : read(gSecondsRegister),
        },
        date : {
            day : read(gDayRegister),
            month : read(gMonthRegister),
            year : read(gYearRegister)
        }
    };
    
    return now;
}

uint8_t RTC::read(uint8_t reg) const {
    outb(gSelectPort, reg | gNMI);
    return inb(gDataPort);
}

void RTC::write(uint8_t reg, uint8_t value) {
    outb(gSelectPort, reg | gNMI);
    outb(gDataPort, value);
}

bool RTC::updating() const {
    return 0 != (0x80 & read(gStatusRegisterA));
}
