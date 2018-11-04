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

#include <kernel/drivers/framebuffer/fb.h>
#include <kernel/log/log.h>
#include <kernel/mm/virt.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>

#include <xnu_font/font.c>

LOG_TAG(POSITION, 2);

#ifndef FONT_WIDTH
	#ifdef ISO_CHAR_WIDTH
		#define FONT_WIDTH ISO_CHAR_WIDTH
	#else
		#warning "unknown value of FONT_WIDTH: assuming default 8"
		#define FONT_WIDTH 8
	#endif
#endif
#ifndef FONT_HEIGHT
	#ifdef ISO_CHAR_HEIGHT
		#define FONT_HEIGHT ISO_CHAR_HEIGHT
	#else
		#warning "unknown value of FONT_HEIGHT: assuming default 16"
		#define FONT_HEIGHT 16
	#endif
#endif

static Framebuffer *gFramebuffer = nullptr;
static uint8_t gFramebufferAlloc[sizeof(Framebuffer)];

Framebuffer::color_t Framebuffer::color_t::black() { return color_t{0,0,0}; }
Framebuffer::color_t Framebuffer::color_t::white() { return color_t{255,255,255}; }
Framebuffer::color_t Framebuffer::color_t::red() { return color_t{255,0,0}; }
Framebuffer::color_t Framebuffer::color_t::green() { return color_t{0,255,0}; }
Framebuffer::color_t Framebuffer::color_t::blue(){ return color_t{0,0,255}; }

Framebuffer::color_t::operator uint32_t() const {
	return (r << 16) | (g << 8) | b;
}

Framebuffer::color_t::color_t(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

Framebuffer::color_t::color_t(uint32_t clr) {
	clr &= 0xFFFFFF;
	r = (clr & 0xFF0000) >> 16;
	g = (clr & 0xFF00) >> 8;
	b = (clr & 0xFF);
}

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

    LOG_DEBUG("initialized a framebuffer of %u x %u pixels - pitch = %u, bpp = %u, base = %p; rows by cols = %u x %u",
        mWidth, mHeight, mPitch, mBytesPerPixel, mPhysicalAddress,
		rows(), columns());
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

uint16_t Framebuffer::row() const {
	return mX / FONT_HEIGHT;
}
uint16_t Framebuffer::column() const {
	return mY / FONT_WIDTH;
}

void Framebuffer::setfg(const color_t& c) {
	mForeground = c;
}
void Framebuffer::setbg(const color_t& c, bool recolor) {
	auto oldBackground = mBackground;
	mBackground = c;
	if (recolor) this->recolor(oldBackground, mBackground);
}

void Framebuffer::setRow(uint16_t r) {	
	auto x = r * FONT_HEIGHT;
	if (x > mHeight) x = mHeight;
	TAG_DEBUG(POSITION, "r = %u, x = %u", r, x);
	mX = x;
}
void Framebuffer::setCol(uint16_t c) {
	auto y = c * FONT_WIDTH;
	if (y > mWidth) y = mWidth;
	TAG_DEBUG(POSITION, "c = %u, y = %u", c, y);
	mY = y;
}

void Framebuffer::cls() {
	memset_pattern4((void*)base(), (uint32_t)mBackground, size());
	setRow(0);
	setCol(0);
}

uint8_t* Framebuffer::row(int16_t n) const {
	if (n < 0) {
		return (uint8_t*)(end() - (-n)*FONT_HEIGHT*mPitch);
	} else {
		return (uint8_t*)(base() + n*FONT_HEIGHT*mPitch);
	}
}

void Framebuffer::recolor(const color_t& Old, const color_t& New) {
	uint32_t oldVal = (uint32_t)Old;
	uint32_t newVal = (uint32_t)New;

	for(uint16_t x = 0; x < mHeight; ++x) {
		for (uint16_t y = 0; y < mWidth; ++y) {
			auto p = getpixel(x,y);
			if (*p == oldVal) *p = newVal;
		}
	}
}

void Framebuffer::paint(uint16_t x, uint16_t y, const color_t& color) {
    auto p = getpixel(x, y);
	*p = (uint32_t)color;
}

void Framebuffer::putdata(unsigned char* fontdata, uint16_t start_x, uint16_t start_y, const color_t& color) {
	auto p0 = getpixel(start_x, start_y);
	auto fg = (uint32_t)color;
	auto p = p0;

    for (uint8_t i = 0u; i < FONT_HEIGHT; ++i, p += (mPitch >> 2)) {
		auto&& fdi = fontdata[i];
		auto j = FONT_WIDTH - 1;
		do {
			auto light_up = 0 != (fdi & (1 << j));
			if (light_up) p[j * (mBytesPerPixel >> 2)] = fg;
			if (j == 0) break;
			--j;
		} while(true);
    }
}

void Framebuffer::putchar(uint16_t start_x, uint16_t start_y, uint8_t chr, const color_t& color) {
    auto fontdata = &iso_font[chr * FONT_HEIGHT];
	putdata(fontdata, start_x, start_y, color);
}

void Framebuffer::clearAtCursor() {
	// we know FONT_HEIGHT to be 16 - but in the interest of pretending to be generalizing, just write *a lot* of extra bytes anyway
	// (this is cheap because we always pass by pointer + it's just 40 bytes anyway & we're on a PC); as long as the size of the array
	// is at least as large as FONT_HEIGHT we will be fine here; and the assert below will tell us if we ever need to add more
	static unsigned char gFiller[] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
	static_assert(sizeof(gFiller) >= FONT_HEIGHT);

	putdata(gFiller, mX, mY, mBackground);
}

void Framebuffer::clearLine(bool to_cursor, bool from_cursor) {
	auto col0 = column();
	if (to_cursor) {
		for(uint16_t col = 0; col <= col0; ++col) {
			setCol(col);
			clearAtCursor();
		}
		setCol(col0);
	}
	if (from_cursor) {
		for(uint16_t col = col0; col < columns(); ++col) {
			setCol(col);
			clearAtCursor();
		}
		setCol(col0);
	}
}

void Framebuffer::rewind(bool erase) {
	if (mY > 0) {
		mY -= FONT_WIDTH;
	} else if (mX > 0) {
		mY = FONT_WIDTH * ((mWidth / FONT_WIDTH) - 2);
		mX -= FONT_HEIGHT;
	}

	if (erase) clearAtCursor();
}

void Framebuffer::advance() {
	mY += FONT_WIDTH;
	if ((mY + FONT_WIDTH) >= mWidth) nl();
}

void Framebuffer::linefeed() {
	mY = 0;
}

void Framebuffer::nl() {
	mX += FONT_HEIGHT;
	mY = 0;

	if((mX + FONT_HEIGHT) > mHeight) {
		uint8_t* row0 = row(0);
		uint8_t* row1 = row(1);
		uint8_t* rowlast = row(-1);
		auto sz = size() - (FONT_HEIGHT * mPitch);
		memcpy(row0, row1, sz);
		memset_pattern4((void*)rowlast, (uint32_t)mBackground, FONT_HEIGHT * mPitch);
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

Framebuffer& Framebuffer::putc(char c) {
	return putc(c, mForeground);
}

Framebuffer& Framebuffer::putc(char c, const color_t& color) {
	switch (c) {
		case '\n':
			nl();
			break;
		case '\b':
			rewind(true);
			break;
		case '\r':
			linefeed();
			break;
		default:
			clearAtCursor();
			putchar(mX, mY, c, color);
			advance();
			break;
	}
	return *this;
}

Framebuffer& Framebuffer::write(const char* s) {
    return write(s, mForeground);
}

Framebuffer& Framebuffer::write(const char* s, const color_t& color) {
	if (s != nullptr) {
		while(auto c = *s) {
			putc(c, color);
			++s;
		}
	}

	return *this;
}
