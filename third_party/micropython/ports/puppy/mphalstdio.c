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

#include <unistd.h>
#include "py/mpconfig.h"

int mp_hal_stdin_rx_chr(void) {
    unsigned char c = 0;
    int r = read(0, &c, 1);
    (void)r;
    return c;
}

void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    int r = write(1, str, len);
    (void)r;
}

void mp_hal_stdout_tx_strn_cooked (const char *str, size_t len) {
    mp_hal_stdout_tx_strn(str, len);
}
