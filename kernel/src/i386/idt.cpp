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

#include <kernel/i386/idt.h>
#include <kernel/i386/cpustate.h>
#include <kernel/i386/primitives.h>
#include <kernel/libc/string.h>
#include <kernel/log/log.h>
#include <muzzle/string.h>
#include <kernel/process/manager.h>
#include <kernel/process/current.h>
#include <kernel/synch/waitqueue.h>

LOG_TAG(INIRQ, 2);
LOG_TAG(IRQSETUP, 1);

Interrupts::handler_t::handler_t() : func(nullptr), payload(nullptr), count(0) {
    bzero(name, sizeof(name));
}

Interrupts::handler_t::operator bool() {
    return func != nullptr;
}

static uint32_t gIRQDepthCounter = 0;

extern "C"
void interrupt_handler(GPR gpr, InterruptStack stack) {
    __atomic_add_fetch(&gIRQDepthCounter, 1, __ATOMIC_SEQ_CST);
    bool yield_on_exit = false;

    TAG_DEBUG(INIRQ, "received IRQ %u", stack.irqnumber);
    auto& handler = Interrupts::get().mHandlers[stack.irqnumber];
    handler.count += 1;
    TAG_DEBUG(INIRQ, "IRQ %u occurred %llu times", stack.irqnumber, handler.count);
	if (handler) {
		auto action = handler.func(gpr, stack, handler.payload);
        if ((action & IRQ_RESPONSE_WAKE) == IRQ_RESPONSE_WAKE) {
            if (handler.wq == nullptr) {
                LOG_ERROR("wake requested but IRQ has no waitqueue");
            } else {
                handler.wq->wakeall();
            }
        }
        if ((action & IRQ_RESPONSE_YIELD) == IRQ_RESPONSE_YIELD) {
            if (nullptr == gCurrentProcess) {
                LOG_ERROR("yield requested outside of process context");
            } else {
                yield_on_exit = true;
            }
        }
	} else {
        TAG_DEBUG(INIRQ, "IRQ %u received - no handler", stack.irqnumber);
    }

    __atomic_fetch_sub(&gIRQDepthCounter, 1, __ATOMIC_SEQ_CST);
    if (yield_on_exit) ProcessManager::get().yield();
}

uint32_t Interrupts::irqDepth() const {
    return __atomic_load_n(&gIRQDepthCounter, __ATOMIC_SEQ_CST);
}

Interrupts& Interrupts::get() {
	static Interrupts gInterrupts;
	
	return gInterrupts;
}

uint64_t Interrupts::getNumOccurrences(uint8_t irq) {
    return mHandlers[irq].count;
}

const char* Interrupts::getName(uint8_t irq) {
    return &mHandlers[irq].name[0];
}

static struct {
	uint16_t size;
	uintptr_t offset;
} __attribute__((packed)) gIDT;

static_assert(sizeof(gIDT) == 6, "IDT reference not 6 bytes in size");

void Interrupts::install() {
    gIDT = {sizeof(mEntries)-1, (uintptr_t)&mEntries};
    loadidt((uintptr_t)&gIDT);
}

void Interrupts::enable() {
    if (mCliCount == 0) {
        mCliCount = 1;
        enableirq();
    } else if (mCliCount < 0) {
        ++mCliCount;
    }
}
void Interrupts::disable() {
    if (mCliCount == 1) {
        disableirq();
        mCliCount = 0;
    } else if (mCliCount <= 0) {
        --mCliCount;
    }
}

bool Interrupts::enabled() {
    return 0 != (readflags() & 512);
}

Interrupts::ScopedDisabler::ScopedDisabler() {
    Interrupts::get().disable();
}
Interrupts::ScopedDisabler::~ScopedDisabler() {
    Interrupts::get().enable();
}
Interrupts::ScopedDisabler::operator bool() {
    return false == Interrupts::get().enabled();
}

void Interrupts::sethandler(uint8_t irq, const char* name, handler_t::irq_handler_f f, void* payload, WaitQueue* wq) {
    TAG_INFO(IRQSETUP, "function at 0x%p set as handler for irq %d", f, irq);
    auto& handler = mHandlers[irq];
    handler.payload = payload;
    handler.wq = wq;
    bzero(handler.name, sizeof(handler.name));
    strncpy(handler.name, name, sizeof(handler.name) - 1);
	handler.func = f;
}

void Interrupts::setWakeQueue(uint8_t irq, WaitQueue* wq) {
    auto& handler = mHandlers[irq];
    handler.wq = wq;
}
