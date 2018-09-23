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

#include <kernel/libc/time.h>

static constexpr uint64_t gSecondsInDay = 60 /* secs/min */ * 60 /* min/hr */ * 24 /* hr */;

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

uint64_t date_components_to_epoch(date_components_t cs) {
    uint64_t val = 0;

    const bool is_leap = leap(cs.year);

    // add up all years until the current one
    for (auto y = 1970u; y < cs.year; ++y) {
        val += gSecondsInDay * (leap(y) ? 366 : 365);
    }

    // for the current year, add up months fully elapsed
    for (auto m = 1; m < cs.month; ++m) {
        val += gSecondsInDay * m[is_leap ? gLeapDays : gDays];
    }

    // for the current month, add up days fully elapsed
    val += (cs.day - 1) * gSecondsInDay;

    return val;
}

uint64_t time_components_to_epoch(time_components_t cs) {
    return cs.second + (60 * (cs.minute + (60 * cs.hour)));
}
