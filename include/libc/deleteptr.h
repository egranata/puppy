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

#ifndef LIBC_DELETEPTR
#define LIBC_DELETEPTR

#include <libc/memory.h>
#include <sys/nocopy.h>

template<typename T>
class delete_ptr : NOCOPY {
    public:
        typedef void(*deletef)(T* ptr);

    delete_ptr(T* stuff) { reset(stuff); }
    delete_ptr(T stuff, deletef del) { reset(stuff, del); }

    T* operator->() { return mData; }

    T* get() { return mData; }

    const T* get() const { return mData; }

    explicit operator bool() { return mData != nullptr; }

    explicit operator T*() { return mData; }

    T* reset(T* stuff = nullptr, deletef del = nullptr) {
        if (del == nullptr) {
            del = [] (T* ptr) { free(ptr); };
        }

        auto old = mData;
        mData = stuff;
        mDeleter = del;
        
        return old;
    }

    ~delete_ptr() {
        mDeleter(mData);
        mData = nullptr;
        mDeleter = nullptr;
    }

    private:
        T* mData;
        deletef mDeleter;
};

#endif
