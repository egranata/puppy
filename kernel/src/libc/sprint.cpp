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

#include <kernel/libc/sprint.h>
#include <stdarg.h>
#include <kernel/libc/pair.h>
#include <kernel/libc/string.h>

class Impl {
private:
	template<typename T>
	struct opt : public pair<T, bool> {
		opt() : pair<T, bool>(T(), false) {}
		opt(T value) : pair<T, bool>(value, true) {}
		
		explicit operator bool() {
			return this->pair<T, bool>::second;
		}

		bool operator==(const T& i) {
			if (this->pair<T, bool>::second) {
				return i == this->pair<T, bool>::first;
			} else {
				return false;
			}
		}
	};
	
	char* mDestBuffer;
	size_t mDestLength;
	const char* mSrcFormat;
	va_list mArgs;
	uint8_t mMode;

public:
	Impl(char* dest, size_t max, const char* fmt, va_list args) : mDestBuffer(dest), mDestLength(max), mSrcFormat(fmt), mArgs(args), mMode(1) {}
	
	size_t putc(char c) {
		if (eob()) {
			return 0;
		} else {
			*mDestBuffer = c;
			++mDestBuffer;
			--mDestLength;
			return 1;
		}
	}
	
	size_t puts(const char* s) {
		size_t count = 0;
		while(*s) {
			if (eob()) break;
			count += putc(*s);
			++s;
		}
		return count;
	}

	size_t putl(uint64_t n) {
		char buffer[22];
		auto bufptr = num2str(n, &buffer[0], 22);
		return puts(bufptr);
	}

	size_t putlx(uint64_t n) {
		char buffer[25];
		auto bufptr = num2str(n, &buffer[0], 24, 16, false);
		return puts("0x") + puts(bufptr);
	}

	size_t putn(signed int n) {
		char buffer[13];
		auto bufptr = num2str(n, &buffer[0], 13);
		return puts(bufptr);
	}

	size_t putu(unsigned int n) {
		char buffer[13];
		auto bufptr = num2str(n, &buffer[0], 13);
		return puts(bufptr);
	}

	size_t putx(unsigned int n) {
		char buffer[13];
		auto bufptr = num2str(n, &buffer[0], 13, 16);
		return puts(bufptr);
	}
	
	size_t putp(uintptr_t p) {
		return p ? putx(p) : puts("<nullptr>");
	}
	
	bool has(size_t n) {
		return mDestLength >= n;
	}
	
	bool eob() {
		return mDestLength == 0;
	}
	
#define PUTC(x) { count += putc(x); if (eob()) { goto done; } }
#define INVERT mMode = -mMode + 3
	
	size_t print() {
		size_t count = 0;
		while(opt<char> nc = next()) {
			switch (nc.first) {
				case '%':
					switch (mMode) {
						case 1: INVERT; break;
						case 2: PUTC('%'); INVERT; break;
					}
					break;
				default:
					switch (mMode) {
						case 1: PUTC(nc.first); break;
						case 2:
						INVERT;
						switch (nc.first) {
							case 's': {
								char* buf = va_arg(mArgs, char*);
								count += puts(buf ? buf : "<null>");
								if (eob()) { goto done; }
								break;
							}
							case 'd': {
								signed int n = va_arg(mArgs, signed int);
								count += putn(n);
								if (eob()) { goto done; }
								break;
							}
							case 'u': {
								unsigned int n = va_arg(mArgs, unsigned int);
								count += putu(n);
								if (eob()) { goto done; }
								break;
							}
							case 'p': {
								uintptr_t p = va_arg(mArgs, uintptr_t);
								count += putp(p);
								if (eob()) { goto done; }
								break;
							}
							case 'x': {
								unsigned int p = va_arg(mArgs, unsigned int);
								count += putx(p);
								if (eob()) { goto done; }
								break;
							}
							case 'c': {
								char c = (char)va_arg(mArgs, unsigned int);
								PUTC(c);
								break;
							}
							case 'l': {
								if (peek(0) == 'u') {
									next();
									unsigned long int n = va_arg(mArgs, unsigned long int);
									count += putl(n);
									if (eob()) { goto done; }
								} else {
									if (peek(0) == 'l') {
										if (peek(1) == 'u') {
											next(); next();							
											uint64_t n = va_arg(mArgs, uint64_t);
											count += putl(n);
											if (eob()) { goto done; }
										} else if (peek(1) == 'x') {
											next(); next();
											uint64_t n = va_arg(mArgs, uint64_t);
											count += putlx(n);
											if (eob()) { goto done; }
										} else {
											goto done;
										}
									} else {
										goto done;
									}
								}

								break;
							}
							default: PUTC(nc.first); break;
						}
					}
					break;
			}
		}
		done:
			*mDestBuffer = 0;
			return count;
	}
	
	opt<char> next() {
		if (!*mSrcFormat) { return opt<char>(); } else {
			opt<char> r = opt<char>(*mSrcFormat);
			return ++mSrcFormat, r;			
		}
	}

	opt<char> peek(size_t i = 0) {
		if (!*mSrcFormat) { return opt<char>(); } else {
			auto p = mSrcFormat;
			while(i > 0) {
				if (*p == 0) return opt<char>();
				--i, ++p;
			}
			return *p ? *p : opt<char>();
		}
	}
};

extern "C"
size_t vsprint(char* dest, size_t max, const char* fmt, va_list args) {
		return Impl{dest, max, fmt, args}.print();
}

extern "C"
size_t sprint(char* dest, size_t max, const char* fmt, ...) {
	if(dest == nullptr || max == 0) {
		return 0;
	}
    va_list argptr;
	
    va_start( argptr, fmt );
	
	auto ret = vsprint(dest, max, fmt, argptr);
	
	va_end ( argptr );
	
	return ret;
}
