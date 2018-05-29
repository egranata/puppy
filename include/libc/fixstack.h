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

#ifndef LIBC_FIXSTACK
#define LIBC_FIXSTACK

#include <sys/stdint.h>

template<typename T, size_t N>
class FixSizeStack {
    public:
        FixSizeStack() : mLen(0) {}

        void push(const T& item) {
            mItems[mLen++] = item;
        }

        T pop() {
            return mItems[--mLen];
        }

        explicit operator bool() {
            return mLen > 0 && mLen < N;
        }

        bool empty() const {
            return mLen == 0;
        }

        bool full() const {
            return mLen == (N - 1);
        }

        size_t len() const {
            return mLen;
        }

        const T& peek() {
            return mItems[mLen - 1];
        }

    private:
        T mItems[N];
        size_t mLen;
};

#endif
