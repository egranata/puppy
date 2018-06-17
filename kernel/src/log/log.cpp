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

#include <kernel/log/log.h>
#include <kernel/libc/sprint.h>
#include <kernel/drivers/serial/serial.h>
#include <kernel/drivers/pit/pit.h>
#include <kernel/sys/config.h>

extern "C"
void __really_log(const char* tag, const char* filename, unsigned long line, const char* fmt, va_list args) {
    static constexpr size_t gBufferSize = 1024;    
    static char gBuffer[gBufferSize];
    size_t n = 0;
    if (tag) {
		    n = sprint(&gBuffer[0], gBufferSize, "[%llu] %s:%lu (%s) ", PIT::getUptime(), filename, line, tag);
    } else {
		    n = sprint(&gBuffer[0], gBufferSize, "[%llu] %s:%lu ", PIT::getUptime(), filename, line);
    }
	  vsprint(&gBuffer[n], gBufferSize-n, fmt, args);
    Serial::get().write(gBuffer).write("\n");
}
