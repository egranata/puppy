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

#include "../include/ansi.h"

ansi_escape_t::ansi_escape_t(const char* s) {
    mString.append_sprintf("%s", s);
}

ansi_escape_t ansi_escape_t::reset() {
    return ansi_escape_t ("\x1b[0m");
}

ansi_escape_t ansi_escape_t::foreground(const color_t& c) {
    std::string s;
    s.append_sprintf("\x1b[38;2;%u;%u;%um",
        c.red, c.green, c.blue);
    return ansi_escape_t(s.c_str());
}
ansi_escape_t ansi_escape_t::background(const color_t& c) {
    std::string s;
    s.append_sprintf("\x1b[48;2;%u;%u;%um",
        c.red, c.green, c.blue);
    return ansi_escape_t(s.c_str());
}

const char* ansi_escape_t::c_str() const {
    return mString.c_str();
}

