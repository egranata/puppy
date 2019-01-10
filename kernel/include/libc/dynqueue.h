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

#ifndef LIBC_DYNQUEUE
#define LIBC_DYNQUEUE

#include <kernel/libc/vec.h>

template<typename T>
class dynqueue {
    public:
        bool empty() const {
            return mElements.empty();
        }

        size_t size() const {
            return mElements.size();
        }

        void push(const T& t) {
            mElements.push_back(t);
        }

        T pop() {
            auto ret = mElements.front();
            mElements.erase(mElements.begin());
            return ret;
        }

        void foreach(bool(*f)(T&)) {
            while(!empty()) {
                auto thingy = pop();
                if(false == f(thingy)) break;
            }
        }

        T peek() {
            return mElements.front();
        }

        void remove(const T& t) {
            mElements.eraseAll(t);
        }

        template<typename U>
        void remove(bool(*f)(const T& t, U u), U u) {
            mElements.eraseAll(f, u);
        }

    private:
        vector<T> mElements;
};

#endif
