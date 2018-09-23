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

#ifndef DRIVERS_RTC_RTC
#define DRIVERS_RTC_RTC

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>

class RTC : NOCOPY {
    public:
        static constexpr uint8_t gIRQNumber = 8;
        
        struct cmos_now_t {
            struct time_t {
                uint8_t hour;
                uint8_t minute;
                uint8_t second;

                bool operator==(const time_t&) const;
            } time;
            struct date_t {
                uint8_t day;
                uint8_t month;
                uint16_t year;

                bool operator==(const date_t&) const;
            } date;

            bool operator==(const cmos_now_t&) const;
            void tick();
            uint64_t timestamp() const;
        };
        static RTC& get();

        uint64_t timestamp() const;

        static constexpr uint16_t gSelectPort = 0x70;
        static constexpr uint16_t gDataPort = 0x71;

        static constexpr uint8_t gNMI = 0x80;

        static constexpr uint8_t gStatusRegisterC = 0x0C;
    private:
        

        static constexpr uint8_t gSecondsRegister = 0x00;
        static constexpr uint8_t gMinutesRegister = 0x02;
        static constexpr uint8_t gHoursRegister = 0x04;
        static constexpr uint8_t gDayRegister = 0x07;
        static constexpr uint8_t gMonthRegister = 0x08;
        static constexpr uint8_t gYearRegister = 0x09;

        static constexpr uint8_t gStatusRegisterA = 0x0A;
        static constexpr uint8_t gStatusRegisterB = 0x0B;

        cmos_now_t now();

        uint8_t read(uint8_t reg) const;
        void write(uint8_t reg, uint8_t value);

        bool updating() const;

        RTC();
};

#endif
