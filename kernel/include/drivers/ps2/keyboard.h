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

class PS2Keyboard : public PS2Controller::Device {
    public:
        struct key_event_t {
            static constexpr uint16_t UP    = 0x0165;
            static constexpr uint16_t LFT   = 0x0166;
            static constexpr uint16_t RHT   = 0x0167;
            static constexpr uint16_t DWN   = 0x0168;

            static constexpr uint16_t F1         = 0x0265;
            static constexpr uint16_t F2         = 0x0266;
            static constexpr uint16_t F3         = 0x0267;
            static constexpr uint16_t F4         = 0x0268;
            static constexpr uint16_t F5         = 0x0269;
            static constexpr uint16_t F6         = 0x0270;
            static constexpr uint16_t F7         = 0x0271;
            static constexpr uint16_t F8         = 0x0272;
            static constexpr uint16_t F9         = 0x0273;
            static constexpr uint16_t F10        = 0x0274;
            static constexpr uint16_t F11        = 0x0275;
            static constexpr uint16_t F12        = 0x0276;

            enum class keymap_to_use : uint8_t {
                CAPS_LOCK,
                SHIFT,
                LOWERCASE,
                LONG
            };

            keymap_to_use keymap;
            bool ctrldown;
            bool altdown;
            uint16_t keycode;
            bool down;

            key_event_t();
        };

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
