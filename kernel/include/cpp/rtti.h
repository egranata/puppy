/*
 * Copyright 2019 Google LLC
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

#ifndef CPP_RTTI
#define CPP_RTTI

#include <kernel/panic/panic.h>

template<typename D, typename B>
bool rtti_isa(const B* b) {
    return b && D::classof(b);
}

template<typename D, typename B>
D* rtti_trycast(const B* b) {
    if (rtti_isa<D,B>(b)) return (D*)b;
    return nullptr;
}

template<typename D, typename B>
D* rtti_cast(const B* b) {
    if (rtti_isa<D,B>(b)) return (D*)b;
    PANIC("invalid RTTI cast");
}

#endif
