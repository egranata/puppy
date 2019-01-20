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

#ifndef NEWLIB_SCOPEDPTR
#define NEWLIB_SCOPEDPTR

namespace newlib::puppy::impl {
    template<typename T>
    struct scoped_ptr_t {
        T* ptr;
        template<typename U = T>
        scoped_ptr_t(U* ptr = nullptr) : ptr((T*)ptr) {}

        scoped_ptr_t(decltype(nullptr) np) : ptr(np) {}

        template<typename U = T>
        scoped_ptr_t<T>& operator=(U*) = delete;

        template<typename U = T>
        scoped_ptr_t(scoped_ptr_t<U>&) = delete;

        template<typename U = T>
        scoped_ptr_t(scoped_ptr_t<U>&& rhs) : ptr(rhs.reset()) {}

        template<typename U = T, typename Q = T>
        Q* reset(U* next = nullptr) {
            auto out = ptr;
            ptr = (T*)next;
            return (Q*)out;
        }

        template<typename U = T>
        U* get() {
            return (U*)ptr;
        }

        template<typename U = T>
        scoped_ptr_t<T>& operator=(const scoped_ptr_t<U>&) = delete;

        ~scoped_ptr_t() {
            free(ptr);
        }
    };
}

#endif
