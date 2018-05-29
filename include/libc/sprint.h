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

#ifndef LIBC_SPRINT
#define LIBC_SPRINT

#include <stdarg.h>
#include <sys/stdint.h>

extern "C"
size_t sprint(char* dest, size_t max, const char* fmt, ...) __attribute__ ((format (printf, 3, 4)));

extern "C"
size_t vsprint(char* dest, size_t max, const char* fmt, va_list args)__attribute__ ((format (printf, 3, 0)));

#endif
