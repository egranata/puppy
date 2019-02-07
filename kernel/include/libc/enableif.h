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

#ifndef LIBC_ENABLEIF
#define LIBC_ENABLEIF

template<bool value, typename T=void>
struct enable_if;

template<typename T>
struct enable_if<true, T> {
    using type = T;
};

template<typename T>
struct enable_if<false, T> {};

template<typename T, typename U>
struct is_same {
    static constexpr bool value = false;
};
 
template<typename T>
struct is_same<T, T> {
    static constexpr bool value = true;
};

template<typename Q>
struct negate {
    static constexpr bool value = !Q::value;
};

template<typename T>
struct is_void : public is_same<T, void> {};

template<typename T>
struct is_not_void : public negate<is_void<T>> {};

static_assert(is_void<void>::value);
static_assert(is_not_void<int>::value);

static_assert(!is_void<int>::value);
static_assert(!is_not_void<void>::value);

#endif
