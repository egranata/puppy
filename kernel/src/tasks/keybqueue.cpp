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

#include <kernel/tasks/keybqueue.h>
#include <kernel/process/manager.h>
#include <kernel/process/current.h>
#include <kernel/tty/tty.h>
#include <kernel/synch/waitqueue.h>
#include <kernel/drivers/ps2/controller.h>
#include <kernel/drivers/ps2/keyboard.h>
#include <kernel/i386/idt.h>
#include <kernel/libc/queue.h>

#define LOG_LEVEL 1
#include <kernel/log/log.h>

namespace tasks::keybqueue {
    static queue<key_event_t, 2048> gKeyEvents;
    static WaitQueue* gKeyIRQQueue;
    static PS2Keyboard *gKeyboard;
    static WaitQueue gEventQueue;

    void prepare() {
        gKeyIRQQueue = nullptr;
        gKeyboard = nullptr;

        auto d1 = PS2Controller::get().getDevice1();
        if (d1 && d1->getType() == PS2Controller::Device::Type::KEYBOARD) {
            gKeyboard = (PS2Keyboard*)d1;
            gKeyIRQQueue = &gKeyboard->queue();
        }
    }

    key_event_t readKey() {
        while (gKeyEvents.empty()) {
            gEventQueue.yield(gCurrentProcess);
        }
        return gKeyEvents.read();
    }

    void task() {
        LOG_INFO("preparing for keyboard input");
        prepare();
        if (!gKeyIRQQueue || !gKeyboard) {
            PANIC("cannot find keyboard");
        }
        while (true) {
            bool any = false;
            {
                Interrupts::ScopedDisabler sd;
                key_event_t evt;
                while (gKeyboard->next(&evt)) {
                    any = true;
                    gKeyEvents.write(evt);
                    LOG_DEBUG("ingested new keyboard event: %s '%c'", evt.down ? "down" : "up", evt.keycode);
                }
                if (any) gEventQueue.wakeall();
            }
            gKeyIRQQueue->yield(gCurrentProcess);
        }
    }
}
