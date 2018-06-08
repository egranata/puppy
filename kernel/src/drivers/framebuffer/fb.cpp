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

#include <drivers/framebuffer/fb.h>
#include <log/log.h>
#include <mm/virt.h>
#include <libc/memory.h>
#include <libc/string.h>

#include "font.cpp"

static Framebuffer *gFramebuffer = nullptr;
static uint8_t gFramebufferAlloc[sizeof(Framebuffer)];

Framebuffer::color_t Framebuffer::color_t::black() { return color_t{0,0,0}; }
Framebuffer::color_t Framebuffer::color_t::white() { return color_t{255,255,255}; }
Framebuffer::color_t Framebuffer::color_t::red() { return color_t{255,0,0}; }
Framebuffer::color_t Framebuffer::color_t::green() { return color_t{0,255,0}; }
Framebuffer::color_t Framebuffer::color_t::blue(){ return color_t{0,0,255}; }

Framebuffer& Framebuffer::get() {
    return *gFramebuffer;
}

Framebuffer* Framebuffer::init(uint16_t width, uint16_t height, uint16_t pitch, uint8_t bpp, uintptr_t phys) {
	LOG_DEBUG("initializing a new framebuffer");
    gFramebuffer = new (gFramebufferAlloc) Framebuffer(width, height, pitch, bpp, phys);

    return gFramebuffer;
}

Framebuffer::Framebuffer(uint16_t width, uint16_t height, uint16_t pitch, uint8_t bpp, uintptr_t phys) :
    mWidth(width), mHeight(height), mPitch(pitch), mBytesPerPixel(bpp / 8), mPhysicalAddress(phys), mAddress(0), mX(0), mY(0) {
    
    mBackground = color_t::black();
    mForeground = color_t::green();

    LOG_DEBUG("initialized a framebuffer of %u x %u pixels - pitch = %u, bpp = %u, base = %p",
        mWidth, mHeight, mPitch, mBytesPerPixel, mPhysicalAddress);
}

uintptr_t Framebuffer::base() const {
    return mAddress;
}

uintptr_t Framebuffer::end() const {
	return base() + size();
}

size_t Framebuffer::size() const {
	return mWidth * mHeight * mBytesPerPixel;
}

uint32_t* Framebuffer::getpixel(uint16_t x, uint16_t y) {
    return (uint32_t*)(mAddress + x*mPitch + y*mBytesPerPixel);
}

Framebuffer::color_t Framebuffer::getfg() {
	return mForeground;
}
Framebuffer::color_t Framebuffer::getbg() {
	return mBackground;
}

uint16_t Framebuffer::width() const {
	return mWidth;
}
uint16_t Framebuffer::height() const {
	return mHeight;
}

uint16_t Framebuffer::rows() const {
	return mHeight / FONT_HEIGHT;
}
uint16_t Framebuffer::columns() const {
	return mWidth / FONT_WIDTH;
}

void Framebuffer::setfg(Framebuffer::color_t c) {
	mForeground = c;
}
void Framebuffer::setbg(Framebuffer::color_t c) {
	mBackground = c;
}

void Framebuffer::setRow(uint16_t r) {
	auto y = r * FONT_HEIGHT;
	if (y > mHeight) y = mHeight;
	mY = y;
}
void Framebuffer::setCol(uint16_t c) {
	auto x = c * FONT_WIDTH;
	if (x > mWidth) x = mWidth;
	mX = x;
}

uint8_t* Framebuffer::row(int16_t n) const {
	if (n < 0) {
		return (uint8_t*)(end() - (-n)*FONT_HEIGHT*mPitch);
	} else {
		return (uint8_t*)(base() + n*FONT_HEIGHT*mPitch);
	}
}

void Framebuffer::paint(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
    auto p = getpixel(x, y);
	*p = b | (g << 8) | (r << 16);
}

void Framebuffer::putchar(uint16_t start_x, uint16_t start_y, uint8_t chr, uint8_t r, uint8_t g, uint8_t b) {
    auto fontdata = &font[chr * FONT_HEIGHT];
	auto p0 = getpixel(start_x, start_y);
	auto fg = b | (g << 8) | (r << 16);
	auto p = p0;

    for (uint8_t i = 0u; i < FONT_HEIGHT; ++i, p += (mPitch >> 2)) {
		auto&& fdi = fontdata[i];
		auto q = p;
        for (uint8_t j = 0u; j < FONT_WIDTH; ++j, q += (mBytesPerPixel >> 2)) {
            if (fdi & (1 << (FONT_WIDTH-1-j))) {
				*q = fg;
            }
        }
    }
}

void Framebuffer::rewind(bool erase) {
	LOG_DEBUG("asked to rewind - erase = %u - old coordinates x=%u y=%u", erase, mX, mY);
	if (mY > 0) {
		mY -= FONT_WIDTH;
	} else if (mX > 0) {
		mY = FONT_WIDTH * ((mWidth / FONT_WIDTH) - 2);
		mX -= FONT_HEIGHT;
	}

	LOG_DEBUG("asked to rewind - new coordinates x=%u y=%u", mX, mY);

	if (erase) {
		putchar(mX, mY, 0xdb, 0, 0, 0);
	}
}

void Framebuffer::advance() {
	mY += FONT_WIDTH;
	if ((mY + FONT_WIDTH) >= mWidth) nl();
}

void Framebuffer::nl() {
	mX += FONT_HEIGHT;
	mY = 0;

	if((mX + FONT_HEIGHT) > mHeight) {
		uint8_t* row0 = row(0);
		uint8_t* row1 = row(1);
		uint8_t* rowlast = row(-1);
		auto sz = size() - (FONT_HEIGHT * mPitch);
		memcopy(row1, row0, sz);
		bzero(rowlast, FONT_HEIGHT * mPitch);
		mX = FONT_HEIGHT*((mHeight / FONT_HEIGHT) -1);
	}
}

uintptr_t Framebuffer::map(uintptr_t vmbase) {
	if (mAddress != 0) {
		LOG_ERROR("framebuffer already mapped at %p - not mapping at %p", mAddress, vmbase);
		return 0;
	}
	LOG_DEBUG("mapping framebuffer starting at virtual base %p", vmbase);
	auto&& vm(VirtualPageManager::get());
	auto b = 0u;
	auto e = size();
	VirtualPageManager::map_options_t opts;
	opts.rw(true).frompmm(false).user(false).clear(true).global(true);
	for (; b <= e; b += VirtualPageManager::gPageSize) {
		vm.map(mPhysicalAddress + b, vmbase + b, opts);
	}
	auto end = vmbase + b;
	mAddress = vmbase;
	LOG_DEBUG("mapping of framebuffer complete at %p", end);
	return end;
}

Framebuffer& Framebuffer::write(const char* s) {
    return write(s, mForeground);
}

Framebuffer& Framebuffer::write(const char* s, Framebuffer::color_t color) {
	if (s != nullptr) {
		while(auto c = *s) {
			switch (c) {
				case '\n':
					nl();
					break;
				case '\b':
					rewind(true);
					break;
				default:
					putchar(mX, mY, c, color.r, color.g, color.b);
					advance();
					break;
			}
			++s;
		}
	}

	return *this;
}
