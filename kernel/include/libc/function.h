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

#ifndef LIBC_FUNCTION
#define LIBC_FUNCTION

#include <kernel/sys/stdint.h>
#include <kernel/libc/forward.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/enableif.h>
#include <kernel/panic/panic.h>

template<typename T>
class function;

template<typename Ret, typename... Args>
class function<Ret(Args...)> {
    private:
        using call_t = Ret(*)(void*, Args&&...);
        using new_t = void(*)(void*,void*);
        using delete_t = void(*)(void*);

        call_t mCall;
        new_t mNew;
        delete_t mDelete;
        struct {
            void* buf;
            size_t size;
        } mData;

        template<typename Callable, typename R = Ret>
        static typename enable_if<is_same<Callable, decltype(nullptr)>::value, R>::type docall(Callable*, Args&&...) {
            PANIC("attempting to call a nullptr function object");
        }

        template<typename Callable, typename R = Ret>
        static typename enable_if<negate<is_same<Callable, decltype(nullptr)>>::value, R>::type docall(Callable* c, Args&&... args) {
            return (*c)(forward<Args&&>(args)...);
        }

        template<typename Callable>
        static void donew(Callable *to, const Callable *from) {
            new (to) Callable(*from);
        }

        template<typename Callable>
        static void dodelete(Callable *c) {
            c->~Callable();
            free(c);
        }

    public:
        function() : mCall(nullptr), mNew(nullptr), mDelete(nullptr), mData{nullptr,0} {}
        function(decltype(nullptr)) = delete;

        template<typename Callable>
        function(Callable c) : mCall((call_t)docall<Callable>), mNew((new_t)donew<Callable>), mDelete((delete_t)dodelete<Callable>), mData{malloc(sizeof(Callable)), sizeof(Callable)} {
            mNew(mData.buf, (void*)&c);
        }

        function(const function& other) {
            mCall = other.mCall;
            mNew = other.mNew;
            mDelete = other.mDelete;
            mData.size = other.mData.size;

            if (mCall) {
                mData.buf = malloc(mData.size);
                mNew(mData.buf, other.mData.buf);
            }
        }

        function<Ret(Args...)>& operator=(const function<Ret(Args...)>& other) {
            if (mData.buf && mData.size && mDelete) {
                mDelete(mData.buf);
            }

            mCall = other.mCall;
            mNew = other.mNew;
            mDelete = other.mDelete;
            mData.size = other.mData.size;

            if (mCall) {
                mData.buf = malloc(mData.size);
                mNew(mData.buf, other.mData.buf);
            }

            return *this;
        }
        function<Ret(Args...)>& operator=(const function<Ret(Args...)>&&) = delete;

        function<Ret(Args...)>& operator=(const decltype(nullptr)&) = delete;
        function<Ret(Args...)>& operator=(const decltype(nullptr)&&) {
            if (mData.buf && mData.size && mDelete) {
                mDelete(mData.buf);
            }

            mCall = nullptr;
            mNew = nullptr;
            mDelete = nullptr;
            mData = {nullptr, 0};

            return *this;
        }

        ~function() {
            if (mData.buf && mData.size) {
                mDelete(mData.buf);
            }
        }

        explicit operator bool() {
            return mCall && mData.buf;
        }

        Ret operator()(Args&&... args) const {
            return mCall(mData.buf, forward<Args&&>(args)...);
        }
};

#endif
