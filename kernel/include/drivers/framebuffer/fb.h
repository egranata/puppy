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

#ifndef DRIVERS_FRAMEBUFFER_FB
#define DRIVERS_FRAMEBUFFER_FB

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>

class Framebuffer : NOCOPY {
    public:
        struct color_t {
            uint8_t r;
            uint8_t g;
            uint8_t b;

            static color_t black();
            static color_t white();
            static color_t red();
            static color_t green();
            static color_t blue();
        };

        static Framebuffer& get();

        static Framebuffer* init(uint16_t width, uint16_t height, uint16_t pitch, uint8_t bpp, uintptr_t phys);

        void paint(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
        Framebuffer& write(const char* s);
        Framebuffer& write(const char* s, color_t color);

        uintptr_t base() const;
        uintptr_t end() const;
        size_t size() const;

        uint16_t width() const;
        uint16_t height() const;

        uint16_t rows() const;
        uint16_t columns() const;

        void setRow(uint16_t);
        void setCol(uint16_t);

        color_t getfg();
        color_t getbg();

        void setfg(color_t c);
        void setbg(color_t c);

        uintptr_t map(uintptr_t base);

    private:
        Framebuffer(uint16_t width, uint16_t height, uint16_t pitch, uint8_t bpp, uintptr_t phys);

        uint32_t* getpixel(uint16_t x, uint16_t y);
        void putchar(uint16_t x, uint16_t y, uint8_t chr, uint8_t r, uint8_t g, uint8_t b);
        uint8_t* row(int16_t n) const;

        void advance();
        void rewind(bool erase);
        void nl();

        uint32_t mPadding[512];
        uint16_t mWidth;
        uint16_t mHeight;
        uint16_t mPitch;
        uint8_t mBytesPerPixel;
        uintptr_t mPhysicalAddress;
        uintptr_t mAddress;
        color_t mBackground;
        color_t mForeground;

        uint16_t mX;
        uint16_t mY;
};

#endif
