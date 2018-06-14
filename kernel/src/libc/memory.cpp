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

#include <kernel/libc/memory.h>

extern "C"
void* memcopy(uint8_t* src, uint8_t* dst, size_t len) {
	auto r = dst;

	while(len & 3) {
		*dst++ = *src++;
		--len;		
	}

	uint32_t *xsrc = (uint32_t*)src;
	uint32_t *xdst = (uint32_t*)dst;
	while(len > 0) {
		*xdst++ = *xsrc++;
		len -= 4;
	}

	return r;
}