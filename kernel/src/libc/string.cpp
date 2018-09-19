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

#include <kernel/libc/string.h>
#include <kernel/libc/memory.h>

extern "C"
char* strdup(const char* str) {
	if (!str) return nullptr;
	auto l = 1 + strlen(str);
	auto n = malloc(l);
	memcopy((uint8_t*)str, (uint8_t*)n, l);
	return (char*)n;
}

extern "C"
const char* strprefix(const char* prefix, const char* full) {
	switch (strncmp(prefix, full, strlen(prefix))) {
		case 0: return full + strlen(prefix);
		default: return nullptr;
	}
}

void memset_pattern4(void *b, uint32_t pattern, size_t len) {
	uint32_t *buffer = (uint32_t*)b;
	while (len >= 4) {
		*buffer = pattern;
		++buffer;
		len -= 4;
	}
	uint8_t *remainder = (uint8_t*)buffer;
	switch (len) {
		case 0: break;
		case 1:
			*remainder = pattern & 0xFF;
			break;
		case 2:
			*remainder = pattern & 0xFF;
			*(remainder+1) = (pattern & 0xFF00) >> 8;
			break;
		case 3:
			*remainder = pattern & 0xFF;
			*(remainder+1) = (pattern & 0xFF00) >> 8;
			*(remainder+2) = (pattern & 0xFF0000) >> 16;
			break;
	}
}
