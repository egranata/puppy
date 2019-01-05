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

#ifndef I386_IDT
#define I386_IDT

#include <kernel/sys/stdint.h>
#include <kernel/i386/cpustate.h>

extern "C"
void interrupt_handler(GPR gpr, InterruptStack stack);

static constexpr uint32_t IRQ_RESPONSE_NONE = 0; /* resume execution */
static constexpr uint32_t IRQ_RESPONSE_YIELD = 1u << 1; /* yield the current process */
static constexpr uint32_t IRQ_RESPONSE_WAKE = 1u << 2; /* wake the WaitQueue for this IRQ */
// add values here for different IRQ responses

class WaitQueue;

class Interrupts {
public:
	struct handler_t {
		using irq_response_t = uint32_t;

		using irq_handler_f = irq_response_t(*)(GPR&, InterruptStack&, void*);
		using irq_name_t = char[16];
		irq_handler_f func;
		irq_name_t name;
		void* payload;
		WaitQueue* wq;
		uint64_t count;
		explicit operator bool();
		handler_t();
	};
	
	static Interrupts& get();
	
	void install();
	
	void enable();
	void disable();

	bool enabled();
	
	void sethandler(uint8_t irq, const char* name, handler_t::irq_handler_f = nullptr, void* = nullptr, WaitQueue* wq = nullptr);
	void setWakeQueue(uint8_t irq, WaitQueue* = nullptr);

	uint64_t getNumOccurrences(uint8_t irq);
	const char* getName(uint8_t irq);
	
	class ScopedDisabler {
		public:
			ScopedDisabler();
			~ScopedDisabler();

			explicit operator bool();
	};
private:
	Interrupts();
	
	Interrupts(const Interrupts&) = delete;
	Interrupts(Interrupts&&) = delete;
	Interrupts operator=(const Interrupts&) = delete;
	
	struct Entry {
	    uint16_t mOffsetLow;
	    uint16_t mSelector;
	    uint8_t mReserved;
	    uint8_t mInfo;
	    uint16_t mOffsetHigh;
		
		// allow default construction
		Entry(uintptr_t handler = 0, bool userspace = false, bool mask = true) {
			mOffsetLow = handler & 0xFFFF;
			mOffsetHigh = (handler & 0xFFFF0000) >> 16;
			mSelector = 8;
			mReserved = 0;
			// interrupt gates (0xE) mask other interrupts;
			// trap gates (0xF) do not
			mInfo = (mask ? 0xE : 0xF) | (userspace ? 0xE0 : 0x80); 
		}
	} __attribute__((packed)) mEntries[256];
	
	handler_t mHandlers[256];

	int32_t mCliCount;
	
	friend void interrupt_handler(GPR, InterruptStack);
	
	static_assert(sizeof(Entry) == 8, "IDT entry not 8 bytes in size");
};

#endif
