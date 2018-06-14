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

#include <kernel/libc/mapping.h>
#include <kernel/mm/virt.h>

Mapping::Mapping(uintptr_t base, size_t len) : mBase(base) {
    auto& vm(VirtualPageManager::get());

    mPageFirst = vm.page(mBase);   
    mPageLast = vm.page(mBase + len);

    for (auto p = mPageFirst; p <= mPageLast; p += VirtualPageManager::gPageSize) {
        vm.map(p, p, VirtualPageManager::map_options_t::kernel());
    }
}

Mapping::~Mapping() {
    auto& vm(VirtualPageManager::get());

    for (auto p = mPageFirst; p <= mPageLast; p += VirtualPageManager::gPageSize) {
        vm.unmap(p);
    }
}
