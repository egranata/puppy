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

#include <kernel/mm/heap.h>
#include <kernel/mm/virt.h>

#include <kernel/log/log.h>

Heap::Heap(uintptr_t low, uintptr_t high) : mRange({low, high}), mCurrent(low) {}
Heap::Heap(Heap::range_t range) : mRange(range), mCurrent(range.low) {}

size_t Heap::blocksize() const {
    // the default size of a block is the size of a VM page
    return VirtualPageManager::gPageSize;
}

uintptr_t Heap::low() const {
    return mRange.low;
}
uintptr_t Heap::current() const {
    return mCurrent;
}
uintptr_t Heap::high() const {
    return mRange.high;
}

size_t Heap::size() const {
    return mCurrent - low();
}
size_t Heap::maxsize() const {
    return high() - low();
}

uintptr_t Heap::sbrk(size_t amount) {
    LOG_DEBUG("sbrk for %u bytes", amount);
    if (amount == 0) {
        return mCurrent;
    }

    auto blksz = blocksize();
    auto cur = mCurrent;

    if (amount % blksz) {
        // round up to a multiple of the block size
        amount = (amount - (amount % blksz)) + blksz;
    }

    LOG_DEBUG("sbrk remapped to a multiple of block size (%u) -> %u", blksz, amount);

    while(amount != 0) {
        if (mCurrent >= high()) {
            return 0;
        }
        auto more = oneblock();
        if (more == 0) {
            return 0;
        }

        LOG_DEBUG("oneblock() OK, returned %p", more);

        amount -= blksz;
        mCurrent += blksz;
    }

    // return the old value of the pointer - this is the base of new usable memory
    LOG_DEBUG("returning %p as the current heap pointer", cur);
    return cur;
}
