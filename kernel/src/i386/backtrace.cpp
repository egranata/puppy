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

#include <kernel/i386/backtrace.h>
#include <kernel/mm/virt.h>

void Backtrace::backtrace(const GPR& gpr, callback f, size_t depth) {
    auto&& vmm(VirtualPageManager::get());
    uint32_t *frame = (uint32_t*)gpr.ebp;

	for (size_t i = 0; i < depth; ++i) {
        if (frame == nullptr) break;
        
        auto eip = frame[1];
        auto fp = frame[0];
        
        if (!f(eip)) break;

        if (!vmm.mapped(fp)) break;
        frame = (uint32_t*)fp;
    }

}
