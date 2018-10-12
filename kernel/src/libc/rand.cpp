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

#include <kernel/libc/rand.h>

Rng::Rng(uint64_t seed) : mValue(0), mWeylSeq(0), mSeed(seed) {}

uint64_t Rng::seed() const {
    return mSeed;
}
void Rng::reseed(uint64_t seed) {
    mValue = mWeylSeq = 0;
    mSeed = seed;
}

uint32_t Rng::next() {
    auto newValue = mValue * mValue;
    mWeylSeq += mSeed;
    newValue += mWeylSeq;
    newValue = (newValue >> 32) | (newValue << 32);
    return (uint32_t)(mValue = newValue);
}
uint32_t Rng::last() const {
    return mValue;
}
