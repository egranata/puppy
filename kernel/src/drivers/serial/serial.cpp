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

#include <kernel/drivers/serial/serial.h>
#include <kernel/i386/primitives.h>
#include <kernel/libc/string.h>

static constexpr uint16_t dataport(uint16_t base) {
	return base;
}
static constexpr uint16_t irqregister(uint16_t base) {
	return base + 1;
}
static constexpr uint16_t fifocommandport(uint16_t base) {
	return base + 2;
}
static constexpr uint16_t linecommandport(uint16_t base) {
	return base + 3;
}
static constexpr uint16_t modemcommandport(uint16_t base) {
	return base + 4;
}
static constexpr uint16_t linestatusport(uint16_t base) {
	return base + 5;
}

Serial& Serial::get() {
	static Serial gSerial;
	
	return gSerial;
}

Serial::Serial() {
	outb(irqregister(gCOM1), 0x00);
	outb(linecommandport(gCOM1), 0x80);
	outb(dataport(gCOM1), 0x01); // divisor is 0x??01
	outb(irqregister(gCOM1), 0x00); // divisor is 0x0001 (115200 baud)
	outb(linecommandport(gCOM1), 0x03); // 8 bits, no parity, 1 stop bit
	outb(fifocommandport(gCOM1), 0xC7); // FIFO, 14-byte threshold
	outb(modemcommandport(gCOM1), 0x0B); // RTS/DSR set
}
Serial& Serial::write(const char* s) {
	for(auto len = strlen(s); len > 0; --len) {
		putchar(*s++);		
	}
	
	return *this;
}

Serial& Serial::putchar(char c) {
	while (0 == (inb(linestatusport(gCOM1)) & 0x20));
	outb(dataport(gCOM1), c);
	
	return *this;
}
