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

#ifndef COLORS_ANSI
#define COLORS_ANSI

#include <libcolors/color.h>
#include <string>

class ansi_escape_t {
    public:
        static ansi_escape_t reset();

        static ansi_escape_t defaultForeground(int ttyfd = 0);
        static ansi_escape_t defaultBackground(int ttyfd = 0);

        static ansi_escape_t foreground(const color_t&);
        static ansi_escape_t background(const color_t&);

        const char* c_str() const;
    private:
        ansi_escape_t(const char*);
        std::string mString;
};

#endif
