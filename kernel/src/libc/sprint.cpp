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

#include <kernel/libc/sprint.h>
#include <stdarg.h>
#include <kernel/libc/pair.h>
#include <kernel/libc/string.h>

extern "C" int Acpivsnprintf (char*, size_t, const char*, va_list);

extern "C"
size_t vsprint(char* dest, size_t max, const char* fmt, va_list args) {
	return Acpivsnprintf(dest, max, fmt, args);
}

extern "C"
size_t sprint(char* dest, size_t max, const char* fmt, ...) {
    if(dest == nullptr || max == 0) return 0;

    va_list argptr;
    va_start( argptr, fmt );
    auto ret = vsprint(dest, max, fmt, argptr);
    va_end ( argptr );

    return ret;
}
