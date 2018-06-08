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

#ifndef LIBC_ATOMIC
#define LIBC_ATOMIC

template<typename T>
class atomic {
    public:
        atomic(T value = T()) {
            __atomic_store_n(&mValue, value, __ATOMIC_SEQ_CST);
        }
        T load() const {
            return __atomic_load_n(&mValue, __ATOMIC_SEQ_CST);
        }
        void store(T value) {
            __atomic_store_n(&mValue, value, __ATOMIC_SEQ_CST);
        }
        T exchange(T value) {
            return __atomic_exchange_n(&mValue, value, __ATOMIC_SEQ_CST);
        }

        bool cmpxchg(T& expected, T wanted) {
            return __atomic_compare_exchange_n(&mValue, &expected, wanted, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        }

        bool operator==(T value) const {
            return load() == value;
        }

        atomic<T>& operator++() {
            __atomic_add_fetch(&mValue, 1, __ATOMIC_SEQ_CST);
            return *this;
        }

        atomic<T>& operator--() {
            __atomic_sub_fetch(&mValue, 1, __ATOMIC_SEQ_CST);
            return *this;
        }
    private:
        T mValue;
};

#endif
