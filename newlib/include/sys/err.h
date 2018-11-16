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

#ifndef NEWLIB_ERR
#define NEWLIB_ERR

#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void warn(const char*, ...);
void warnx(const char*, ...);

void err(int, const char*, ...);
void errx(int, const char*, ...);

void vwarn(const char*, va_list);
void vwarnx(const char*, va_list);

void verr(int, const char*, va_list);
void verrx(int, const char*, va_list);

#ifdef __cplusplus
}
#endif

#endif

