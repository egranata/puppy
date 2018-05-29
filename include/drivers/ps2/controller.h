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

#ifndef DRIVERS_PS2_CONTROLLER
#define DRIVERS_PS2_CONTROLLER

#include <sys/stdint.h>

class PS2Controller {
public:
    class Device {
        public:
            enum class Type {
                KEYBOARD,
                MOUSE
            };
            virtual Type getType() = 0;
        protected:
            Device(uint8_t devid);
            void send(uint8_t devid, uint8_t byte);
            uint8_t receive(uint8_t devid);
    };

    static constexpr uint16_t gDataPort = 0x60;
    static constexpr uint16_t gStatusPort = 0x64;
    static constexpr uint16_t gCommandPort = gStatusPort;

    static PS2Controller& get();

    class StatusRegister {
        public:
            bool outputBufferFull() const;
            bool inputBufferFull() const;
            bool systemFlag() const;
            bool isInputCommand() const;
            bool hasTimeoutError() const;
            bool hasParityError() const;

        private:
            StatusRegister(uint8_t value);
            uint8_t mValue;

            friend class PS2Controller;
    };

   class Configuration {
       public:
           bool firstPortIRQ() const;
           bool secondPortIRQ() const;
           bool systemFlag() const;
           bool firstPortClock() const;
           bool secondPortClock() const;
           bool firstPortTranslation() const;

           void firstPortIRQ(bool);
           void secondPortIRQ(bool);
           void firstPortClock(bool);
           void secondPortClock(bool);
           void firstPortTranslation(bool);
       private:
           Configuration(uint8_t);
           uint8_t mValue;
           
           friend class PS2Controller;
   };

    Device* getDevice1();

    private:
        PS2Controller();

        PS2Controller(const PS2Controller&) = delete;
        PS2Controller(PS2Controller&&) = delete;
        PS2Controller& operator=(const PS2Controller&) = delete;

        static StatusRegister status();
        Configuration configuration();
        Configuration configuration(const Configuration&);

        void disable(bool port1 = true, bool port2 = true);
        void command(uint8_t byte);
        void command(uint8_t byte1, uint8_t byte2);

        static void device1(uint8_t byte);
        static void device2(uint8_t byte);

        static uint8_t response();
        static uint8_t response(uint64_t timeout, uint8_t def = 0);

        Device *mDevice1;

        friend class PS2Controller::Device;
};

#endif
