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

#ifndef DRIVERS_PS2_KEYBOARD
#define DRIVERS_PS2_KEYBOARD

#include <kernel/drivers/ps2/controller.h>
#include <kernel/synch/waitqueue.h>
#include <kernel/tty/keyevent.h>

class PS2Keyboard : public PS2Controller::Device {
    public:
        struct keyb_irq_data_t {
            PS2Keyboard *source;
            int pic_irq_id;
            int cpu_irq_id;
        } irq_data;

        PS2Keyboard(uint8_t devid);

        virtual Device::Type getType() override;
        bool any();
        bool next(key_event_t*);

        WaitQueue& queue();

    private:
        // this queue will be woken when a keystroke comes in
        WaitQueue mWaitForEvent;
};

#endif
