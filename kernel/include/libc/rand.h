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

#ifndef LIBC_RAND
#define LIBC_RAND

#include <stdint.h>

// Random number generator (based upon Middle Square Weyl Sequence)
class Rng {
    public:
        Rng(uint64_t seed = 0xb5ad4eceda1ce2a9ULL);

        uint64_t seed() const;
        void reseed(uint64_t);

        uint32_t next();
        uint32_t last() const;
    private:
        uint64_t mValue;
        uint64_t mWeylSeq;
        uint64_t mSeed;
};

#endif
