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

#ifndef DRIVERS_SERIAL_SERIAL
#define DRIVERS_SERIAL_SERIAL

#include <sys/stdint.h>

class Serial {
public:
	static constexpr uint16_t gCOM1 = 0x3F8;
	
	static Serial& get();
	
	Serial& write(const char* s);
	Serial& putchar(char c);
	
private:
	Serial();
};

Serial& operator<<(Serial&, const char*);
Serial& operator<<(Serial&, char);

#endif
