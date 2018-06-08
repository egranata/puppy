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

#ifndef FS_HANDLETABLE
#define FS_HANDLETABLE

#include <libc/string.h>

template<typename T, size_t N>
class Handletable {
    static_assert(0 == (N & 7));

    static constexpr uint8_t bit(uint8_t i) {
        return 1 << (i & 7);
    }

    public:
        using entry_t = T;

        Handletable() {
            bzero(&mValid[0], sizeof(mValid));
        }
        
        constexpr size_t size() {
            return N;
        }

        bool set(const T& value, size_t& idx) {
            for (auto i = 0u; i < N; ++i) {
                if (!is(i)) {
                    mValid[i / 8] |= bit(i);
                    mData[i] = value;
                    idx = i;
                    return true;
                }
            }
            return false;
        }

        bool is(size_t idx, T* value = nullptr) {
            bool ret = 0 != (mValid[idx / 8] & bit(idx));
            if (ret && value) {
                *value = mData[idx];
            }
            return ret;
        }

        void clear(size_t idx) {
            mValid[idx / 8] &= ~bit(idx);
        }

    private:
        uint8_t mValid[N/8];
        T mData[N];
};

#endif
