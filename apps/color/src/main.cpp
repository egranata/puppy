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

#include <libcolors/color.h>
#include <libcolors/tty.h>
#include <libcolors/configcolors.h>

int main(int argc, char** argv) {
    auto default_colors(config_colors_t::systemConfig());
    tty_color_t tty_config;

    if (argc == 1) {
        printf("%u colors loaded:\n", default_colors.size());
        for (const auto& name: default_colors.keys()) {
            color_t value = color_t::white();
            if (default_colors.get(name.c_str(), value)) {
                printf("%s [r=%u g=%u b=%u]\n",
                    name.c_str(),
                    value.red,
                    value.green,
                    value.blue);
            }
        }
    } else if (argc == 2) {
        color_t value = color_t::white();
        bool ok = default_colors.get(argv[1], value);
        if (ok) {
            tty_config.defaultForeground(value);
        } else {
            printf("error: unable to change TTY foreground color\n");
            exit(2);
        }
    } else if (argc == 3) {
        color_t value = color_t::white();
        bool ok = default_colors.get(argv[2], value);
        if (!ok) {
            printf("error: unable to change TTY colors\n");
            exit(2);
        }

        if (0 == strcmp("fg", argv[1])) {
            tty_config.defaultForeground(value);
        } else if (0 == strcmp("bg", argv[1])) {
            tty_config.defaultBackground(value);
        } else {
            printf("unknown color mode: %s\n", argv[1]);
            exit(3);
        }
    } else {
        printf("usage: color with no arguments prints a list of known color data\n"
               "color <name> sets the foreground color\n"
               "color <fg|bg> <name> sets the foreground or background color\n");
        exit(4);
    }

    return 0;
}
