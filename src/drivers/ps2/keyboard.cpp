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

#include <drivers/ps2/keyboard.h>
#include <drivers/pic/pic.h>
#include <i386/idt.h>
#include <i386/primitives.h>

#define LOG_NODEBUG
#include <log/log.h>

static struct keyb_buffer_t {
    static constexpr uint8_t gBufferSize = 128;
    PS2Keyboard::key_event_t mBuffer[gBufferSize] = {0,false};
    uint8_t mWriteIdx = 0;
    uint8_t mReadIdx = 0;
} gKeyboardBuffer;

static uint8_t gCapslockScancodeMap[256] = {
 // 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f   
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '`',  0x00, // 0x00-0x0f
    0x00, 0x05, 0x01, 0x00, 0x03, 'Q',  '1',  0x00, 0x00, 0x00, 'Z',  'S',  'A',  'W',  '2',  0x00, // 0x10-0x1f
    0x00, 'C',  'X',  'D',  'E',  '4',  '3',  0x00, 0x00, ' ',  'V',  'F',  'T',  'R',  '5',  0x00, // 0x20-0x2f
    0x00, 'N',  'B',  'H',  'G',  'Y',  '6',  0x00, 0x00, 0x00, 'M',  'J',  'U',  '7',  '8',  0x00, // 0x30-0x3f
    0x00, ',',  'K',  'I',  'O',  '0',  '9',  0x00, 0x00, '.',  '/',  'L',  ';',  'P',  '-',  0x00, // 0x40-0x4f
    0x00, 0x00, '\'', 0x00, '[',  '=',  0x00, 0x00, 0x00, 0x02, '\n', ']',  0x00, '\\', 0x00, 0x00, // 0x50-0x5f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\b', 0x00, 0x00, '1',  0x00, '4',  '7',  0x00, 0x00, 0x00, // 0x60-0x6f
    '0',  '.',  '2',  '5',  '6',  '8',  0x00, 0x00, 0x00, '+',  '3',  '-',  '*',  '9',  0x00, 0x00, // 0x70-0x7f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80-0x8f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90-0x9f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa0-0xaf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb0-0xbf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xc0-0xcf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xd0-0xdf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe0-0xef
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xf0-0xff
};

static uint8_t gShiftScancodeMap[256] = {
 // 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f   
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '~',  0x00, // 0x00-0x0f
    0x00, 0x05, 0x01, 0x00, 0x03, 'Q',  '!',  0x00, 0x00, 0x00, 'Z',  'S',  'A',  'W',  '@',  0x00, // 0x10-0x1f
    0x00, 'C',  'X',  'D',  'E',  '$',  '#',  0x00, 0x00, ' ',  'V',  'F',  'T',  'R',  '%',  0x00, // 0x20-0x2f
    0x00, 'N',  'B',  'H',  'G',  'Y',  '^',  0x00, 0x00, 0x00, 'M',  'J',  'U',  '&',  '*',  0x00, // 0x30-0x3f
    0x00, '<',  'K',  'I',  'O',  ')',  '(',  0x00, 0x00, '>',  '?',  'L',  ':',  'P',  '_',  0x00, // 0x40-0x4f
    0x00, 0x00, '"', 0x00, '{',   '+',  0x00, 0x00, 0x00, 0x02, '\n', '}',  0x00, '|',  0x00, 0x00, // 0x50-0x5f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '1',  0x00, '4',  '7',  0x00, 0x00, 0x00, // 0x60-0x6f
    '0',  '>',  '2',  '5',  '6',  '8',  0x00, 0x00, 0x00, '+',  '3',  '-',  '*',  '9',  0x00, 0x00, // 0x70-0x7f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80-0x8f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90-0x9f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa0-0xaf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb0-0xbf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xc0-0xcf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xd0-0xdf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe0-0xef
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xf0-0xff
};

static uint8_t gLowercaseScancodeMap[256] = {
 // 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f   
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '`',  0x00, // 0x00-0x0f
    0x00, 0x05, 0x01, 0x00, 0x03, 'q',  '1',  0x00, 0x00, 0x00, 'z',  's',  'a',  'w',  '2',  0x00, // 0x10-0x1f
    0x00, 'c',  'x',  'd',  'e',  '4',  '3',  0x00, 0x00, ' ',  'v',  'f',  't',  'r',  '5',  0x00, // 0x20-0x2f
    0x00, 'n',  'b',  'h',  'g',  'y',  '6',  0x00, 0x00, 0x00, 'm',  'j',  'u',  '7',  '8',  0x00, // 0x30-0x3f
    0x00, ',',  'k',  'i',  'o',  '0',  '9',  0x00, 0x00, '.',  '/',  'l',  ';',  'p',  '-',  0x00, // 0x40-0x4f
    0x00, 0x00, '\'', 0x00, '[',  '=',  0x00, 0x00, 0x02, 0x00, '\n', ']',  0x00, '\\', 0x00, 0x00, // 0x50-0x5f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\b', 0x00, 0x00, '1',  0x00, '4',  '7',  0x00, 0x00, 0x00, // 0x60-0x6f
    '0',  '.',  '2',  '5',  '6',  '8',  0x00, 0x00, 0x00, '+',  '3',  '-',  '*',  '9',  0x00, 0x00, // 0x70-0x7f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80-0x8f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90-0x9f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa0-0xaf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb0-0xbf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xc0-0xcf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xd0-0xdf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe0-0xef
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xf0-0xff
};

static uint8_t gLongScancodeMap[256] = {
 // 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f   
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x00-0x0f
    0x00, 0x06, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x10-0x1f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x20-0x2f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x30-0x3f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '/',  0x00, 0x00, 0x00, 0x00, 0x00, // 0x40-0x4f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\n', 0x00, 0x00, 0x00, 0x00, 0x00, // 0x50-0x5f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x60-0x6f
    0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x70-0x7f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80-0x8f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90-0x9f
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa0-0xaf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb0-0xbf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xc0-0xcf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xd0-0xdf
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe0-0xef
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xf0-0xff
};

static bool gIsBreakCode;
static bool gIsLong;
static bool gCapsLock;
static bool gShift;

static bool gCtrlDown;
static bool gAltDown;
static bool gDeleteDown;

static void keyboard_irq_handler() {
    LOG_DEBUG("in keyboard IRQ");
    auto b = inb(0x60);

    if (b == 0xE0) {
        gIsLong = true;
    } else if (b == 0xF0) {
        gIsBreakCode = true;
    } else {
        auto& event(gKeyboardBuffer.mBuffer[gKeyboardBuffer.mWriteIdx]);
        event.down = !gIsBreakCode;
        event.keycode = (gIsLong ? gLongScancodeMap :
            gShift ? gShiftScancodeMap :
            gCapsLock ? gCapslockScancodeMap : gLowercaseScancodeMap)[b];

        LOG_DEBUG("event keycode: %u", event.keycode);

        if(keyb_buffer_t::gBufferSize == ++gKeyboardBuffer.mWriteIdx) gKeyboardBuffer.mWriteIdx = 0;

        switch(b) {
            case 0x58:
                if (gIsBreakCode) {
                    gCapsLock = !gCapsLock;
                }
                break;
            case 0x12:
            case 0x59:
                gShift = !gIsBreakCode;
                break;
            case 0x14:
                gCtrlDown = !gIsBreakCode;
                break;
            case 0x11:
                gAltDown = !gIsBreakCode;
                break;
            case 0x71:
                gDeleteDown = gIsLong && !gIsBreakCode;
                break;
        }

        // TODO: should we do anything with CTRL+ALT+DEL
        if (gAltDown && gCtrlDown && gDeleteDown) {
            LOG_DEBUG("3 finger chord is pressed - wow. so M$ much DOS.");
        }

        gIsLong = false;
        gIsBreakCode = false;
    }
    LOG_DEBUG("seen keyboard input %u - mWriteIdx = %u", b, gKeyboardBuffer.mWriteIdx);
}

static void keyboard_irq_handler_1(GPR&, InterruptStack&) {
    keyboard_irq_handler();
    PIC::eoi(1);
}

static void keyboard_irq_handler_12(GPR&, InterruptStack&) {
    keyboard_irq_handler();
    PIC::eoi(12);
}

PS2Keyboard::PS2Keyboard(uint8_t devid) : Device(devid) {
    gIsBreakCode = false;
    gIsLong = false;
    gCapsLock = false;
    gShift = false;

    auto cpu_irq = PIC::gIRQNumber(devid == 1 ? 1 : 12);
    Interrupts::get().sethandler(cpu_irq, devid == 1 ? keyboard_irq_handler_1 : keyboard_irq_handler_12);
    LOG_DEBUG("setup keyboard IRQ handler for irq %u - device %u", cpu_irq, devid);
}

PS2Controller::Device::Type PS2Keyboard::getType() {
    return PS2Controller::Device::Type::KEYBOARD;
}

bool PS2Keyboard::any() {
    return (gKeyboardBuffer.mReadIdx != gKeyboardBuffer.mWriteIdx);
}

PS2Keyboard::key_event_t PS2Keyboard::next() {
    if (any()) {
        auto evt = gKeyboardBuffer.mBuffer[gKeyboardBuffer.mReadIdx];
        if(keyb_buffer_t::gBufferSize == ++gKeyboardBuffer.mReadIdx) gKeyboardBuffer.mReadIdx = 0;
        return evt;
    }
    return {0,false};
}
