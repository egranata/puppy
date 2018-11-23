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

#ifndef LIBC_VEC
#define LIBC_VEC

#include <kernel/sys/stdint.h>
#include <kernel/libc/memory.h>
#include <kernel/sys/nocopy.h>
#include <kernel/libc/function.h>

#include <muzzle/string.h>

template<typename T>
class vector : NOCOPY {
    public:
        using size_type = size_t;

        vector() : mData(nullptr), mCapacity(0), mSize(0) {}

        void push_back(const T& item) {
            if (mSize == mCapacity) {
                realloc(2 * mCapacity + 1);
            }
            mData[mSize++] = item;
        }

        T* data() {
            return mData;
        }

        size_t size() const {
            return mSize;
        }

        size_t capacity() const {
            return mCapacity;
        }

        bool empty() const {
            return mSize == 0;
        }

        typedef T* iterator;
        typedef const T* const_iterator;

        iterator begin() {
            return &mData[0];
        }
        iterator end() {
            return &mData[mSize];
        }

        T& front() {
            return mData[0];
        }

        const T& front() const {
            return mData[0];
        }

        T& back() {
            return mData[mSize - 1];
        }

        const T& back() const {
            return mData[mSize - 1];
        }

        const_iterator begin() const {
            return &mData[0];
        }
        const_iterator end() const {
            return &mData[mSize];
        }

        void erase(iterator i) {
            auto pos = i - mData;
            auto rem = mSize - pos - 1;
            i->~T();
            memmove(&mData[pos], &mData[pos + 1], rem * sizeof(T));
            --mSize;
        }

        bool erase(const T& value) {
            for (auto i = begin(); i != end(); ++i) {
                if (*i == value) {
                    erase(i);
                    return true;
                }
            }

            return false;
        }

        void eraseAll(const T& value) {
            while (erase(value));
        }

        void pop_back() {
            if (mSize > 0) --mSize;
        }

        T& at(size_t idx) {
            return mData[idx];
        }

        T& operator[] (size_t idx) {
            return mData[idx];
        }

        void foreach(function<bool(const T&)> f) const {
            auto&& iter = begin();
            auto&& e = end();
            for (; iter != e; ++iter) {
                if (false == f(*iter)) break;
            }
        }

        void foreach(function<bool(T&)> f) {
            auto iter = begin();
            auto e = end();
            for (; iter != e; ++iter) {
                if (false == f(*iter)) break;
            }
        }

    private:
        T* mData;
        size_t mCapacity;
        size_t mSize;

        void realloc(size_t newSize) {
            T* newdata = allocate<T>(newSize);
            memcpy(newdata, mData, sizeof(T) * mSize);
            mCapacity = newSize;
            free(mData);
            mData = newdata;
        }
};

#endif