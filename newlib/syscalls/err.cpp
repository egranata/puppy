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

#include <newlib/sys/err.h>
#include <newlib/impl/cenv.h>
#include <newlib/stdio.h>
#include <newlib/stdlib.h>

NEWLIB_IMPL_REQUIREMENT void verrx(int eval, const char* fmt, va_list va) {
    fprintf(stderr, "error: ");
    if (fmt) vfprintf(stderr, fmt, va);
    exit(eval);
}

NEWLIB_IMPL_REQUIREMENT void verr(int eval, const char* fmt, va_list va) {
    verrx(eval, fmt, va);
}

NEWLIB_IMPL_REQUIREMENT void err(int eval, const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    verr(eval, fmt, va);
    va_end(va);
}

NEWLIB_IMPL_REQUIREMENT void errx(int eval, const char* fmt, ...)  {
    va_list va;
    va_start(va, fmt);
    verrx(eval, fmt, va);
    va_end(va);
}

NEWLIB_IMPL_REQUIREMENT void vwarnx(const char* fmt, va_list va) {
    fprintf(stderr, "warning: ");
    if (fmt) vfprintf(stderr, fmt, va);
}

NEWLIB_IMPL_REQUIREMENT void vwarn(const char* fmt, va_list va) {
    vwarnx(fmt, va);
}

NEWLIB_IMPL_REQUIREMENT void warn(const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vwarn(fmt, va);
    va_end(va);
}

NEWLIB_IMPL_REQUIREMENT void warnx(const char* fmt, ...)  {
    va_list va;
    va_start(va, fmt);
    vwarnx(fmt, va);
    va_end(va);
}
