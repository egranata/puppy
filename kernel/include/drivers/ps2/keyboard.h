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

class PS2Keyboard : public PS2Controller::Device {
    public:
        struct key_event_t {
            static constexpr uint8_t LEFT_SHIFT = 1;
            static constexpr uint8_t RIGHT_SHIFT = 2;
            static constexpr uint8_t LEFT_CTRL = 3;
            static constexpr uint8_t RIGHT_CTRL = 4;
            static constexpr uint8_t LEFT_ALT = 5;
            static constexpr uint8_t RIGHT_ALT = 6;
            static constexpr uint8_t DELETE = 7;
            static constexpr uint8_t BACKSPACE = 8;

            uint8_t keycode;
            bool down;
        };
        PS2Keyboard(uint8_t devid);

        virtual Device::Type getType() override;
        bool any();
        key_event_t next();
};

#endif
