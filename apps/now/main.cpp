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

#include <libuserspace/printf.h>
#include <libuserspace/time.h>

static bool leap(uint16_t year) {
    if (0 != (year % 4)) return false;
    if (0 != (year % 100)) return true;
    return 0 == (year % 400);    
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

struct now_t {
    uint8_t day;
    uint8_t month;
    uint16_t year;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    now_t(uint64_t ts) {
        // fill in HMS components
        second = ts % 60;
        ts /= 60;
        minute = ts % 60;
        ts /= 60;
        hour = ts % 24;
        ts /= 24;

        // pick the year by incrementing from 1970 until you go over
        year = 1970;
        while(true) {
            uint64_t counter = (leap(year) ? 366 : 365);
            if (ts > counter) {
                ts -= counter;
                ++year;
            }
            else {
                break;
            }
        }

        // we are now under a year, so start scanning months
        month = 1;
        auto& days = leap(year) ? gLeapDays : gDays;
        while(ts >= days[month]) {
            ++month; ts -= days[month];
        }
        
        day = ts + 1;
    }
};

int main(int, const char**) {
    auto n = now();
    auto u = uptime();
    auto d = now_t(n);
    printf("Time since epoch: %llu, aka %02u/%02u/%02u at %02u:%02u:%02u - Time since boot: %llu ms\n",
        n,
        d.month, d.day, d.year, d.hour, d.minute, d.second,
        u);
    return 0;
}
