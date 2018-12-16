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

#ifndef LIBC_STRING
#define LIBC_STRING

#include <kernel/sys/stdint.h>
#include <muzzle/string.h>

extern "C"
bool strprefix(const char* prefix, const char* full, const char **next);

extern "C"
char* strdup(const char* str);

template<typename NumType>
const char* num2str(NumType n, char* buffer, size_t buf_size, uint32_t base = 10, bool uppercase = true) {
    if (buffer == nullptr || buf_size == 0) {
        return nullptr;
    }
	
    buffer[buf_size - 1] = 0;
    if (n == 0) {
        if (buf_size > 1) {
            buffer[buf_size - 2] = '0';
            return &buffer[buf_size - 2];
        } else {
            return nullptr;
        }
    }
	
    auto is_negative = (n < 0);
    if (is_negative) n *= -1;
    auto idx = buf_size - 2;

    while(true) {
        auto digit = n % base;
		n /= base;
        if (digit <= 9) {
            digit = '0' + digit;
        } else {
            digit = (uppercase ? 'A' : 'a') + (digit - 10);
        }
        buffer[idx] = (char)digit;
        if (n == 0) {
            break;
        } else if (idx == 0) {
			return nullptr;
		} else {
			--idx;
        }
    }
    if (is_negative) {
        if (idx > 0) {
            buffer[--idx] = '-';
        } else {
            return nullptr;
        }
    }
    return &buffer[idx];
}

void memset_pattern4(void *b, uint32_t pattern, size_t len);

#endif
