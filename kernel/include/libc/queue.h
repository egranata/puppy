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

#ifndef LIBC_QUEUE
#define LIBC_QUEUE

#include <sys/stdint.h>
#include <libc/string.h>

template<typename T>
static constexpr T incr(T n, T max, T wrap = 0) {
	return ++n, (n == max) ? wrap : n;
}

template<class T, size_t N>
class queue {
public:
	queue() : mReadIdx(0), mWriteIdx(0) {
		bzero((uint8_t*)&mItems[0], sizeof(mItems));
	}
	bool empty() { return mReadIdx == mWriteIdx; }
	bool full() { return incr(mWriteIdx, N) == mReadIdx; }
	void write(const T& thing) {
		mItems[mWriteIdx] = thing;
		mWriteIdx = incr(mWriteIdx, N);
	}
	T read() {
		auto t = mItems[mReadIdx];
		mReadIdx = incr(mReadIdx, N);
		return t;
	}
	
private:
	T mItems[N];
	size_t mReadIdx;
	size_t mWriteIdx;
};


#endif
