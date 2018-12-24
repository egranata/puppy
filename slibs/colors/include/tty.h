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

#ifndef COLORS_TTY
#define COLORS_TTY

#include <libcolors/color.h>

class tty_color_t {
    public:
        tty_color_t(int fd = 0);

        color_t defaultBackground();
        color_t defaultForeground();

        void defaultBackground(const color_t&);
        void defaultForeground(const color_t&);
    private:
        int mTTYfd;
};

#endif
