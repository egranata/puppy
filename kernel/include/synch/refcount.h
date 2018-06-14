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

#ifndef SYNCH_REFCOUNT
#define SYNCH_REFCOUNT

#include <kernel/sys/stdint.h>

template <typename T>
class refcountedptr {
    public:
        refcountedptr(T* thing = nullptr) : mThing(thing) {
            incref();
        }

        refcountedptr(refcountedptr<T>&& thing) : mThing(thing.mThing) {
            thing.mThing = nullptr;
        }
        
        refcountedptr(const refcountedptr<T>& thing) : mThing(thing.mThing) {
            incref();
        }

        ~refcountedptr() {
            decref();
        }

        T* operator*() {
            return mThing;
        }

        T* operator->() {
            return mThing;
        }

        explicit operator bool() {
            return mThing != nullptr;
        }

        void reset(T* thing = nullptr) {
            decref();
            mThing = thing;
            incref();
        }

        refcountedptr<T>& operator=(const refcountedptr<T>& thing) {
            reset(thing.mThing);
            return *this;
        }

        refcountedptr<T>& operator=(refcountedptr<T>&& thing) {
            reset(thing.mThing);
            thing.mThing = nullptr;
            return *this;
        }

    private:
        void incref() {
            if (mThing) mThing->incref();
        }

        void decref() {
            if (mThing) mThing->decref();
        }

        T* mThing;
};

template<typename T>
class refcounted {
    protected:
        refcounted() = default;

        void incref() {
            ++mRefCount;
        }
        
        void decref() {
            if (mRefCount) {
                if (--mRefCount == 0) {
                    delete (T*)this;
                }
            }
        }

    private:
        refcounted(const refcounted<T>&) = delete;
        refcounted(refcounted<T>&&) = delete;
        refcounted<T>& operator=(const refcounted<T>&) = delete;

        uint32_t mRefCount;

        friend class refcountedptr<T>;
};

#endif
