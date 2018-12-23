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

#include "../include/color.h"

color_t::color_t(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

color_t color_t::grey(uint8_t n) {
    return color_t(n,n,n);
}

color_t color_t::black() {
    return color_t::grey(0);
}

bool color_t::operator==(const color_t& rhs) const {
    if (rhs.red != red) return false;
    if (rhs.green != green) return false;
    if (rhs.blue != blue) return false;

    return true;
}

color_t color_t::inverted() const {
    return color_t(255-red, 255-green, 255-blue);
}
