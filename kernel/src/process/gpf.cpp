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

#include <stdint.h>
#include <kernel/process/exceptions.h>
#include <kernel/panic/panic.h>
#include <kernel/mm/virt.h>
#include <kernel/log/log.h>

LOG_TAG(GPFHANDLER, 0);

#define GPF_DESCRIPTION "general protection fault"

static constexpr size_t gpf_recovery_count = 32;

static struct {
    uintptr_t eip[gpf_recovery_count];
    size_t idx;
} gpf_recovery_data;

static bool isAllowableRecovery(uintptr_t eip) {
    return (eip != 0) && VirtualPageManager::iskernel(eip);
}

extern "C" void pushGPFRecovery(uintptr_t eip) {
    if (!isAllowableRecovery(eip)) {
        PANIC("GPF recovery frame must be a valid kernel address");
    }
    if (gpf_recovery_data.idx == gpf_recovery_count) {
        PANIC("cannot push new GPF recovery frame");
    }
    TAG_DEBUG(GPFHANDLER, "at index %u pushing recovery %p", gpf_recovery_data.idx, eip);
    gpf_recovery_data.eip[gpf_recovery_data.idx++] = eip;
}

extern "C" void popGPFRecovery(uintptr_t eip) {
    if (gpf_recovery_data.idx == 0) {
        PANIC("cannot remove GPF recovery frame from empty set");
    }
    if (gpf_recovery_data.eip[--gpf_recovery_data.idx] != eip) {
        PANIC("mismatched GPF recovery push/pop");
    }
    gpf_recovery_data.eip[gpf_recovery_data.idx] = 0;
    TAG_DEBUG(GPFHANDLER, "at index %u popped recovery %p", gpf_recovery_data.idx, eip);
}

extern "C" void GPF_handler(GPR& gpr, InterruptStack& stack, void*) {
    if (isKernelStack(stack)) {
        bool can_recover = false;
        uintptr_t eip = 0;
        can_recover = (gpf_recovery_data.idx > 0);
        if (can_recover) {
            // TODO: one should not be allowed to retry the same recovery more than N times
            eip = gpf_recovery_data.eip[gpf_recovery_data.idx - 1];
            if (!isAllowableRecovery(eip)) can_recover = false;
        }
        if (can_recover) {
            TAG_DEBUG(GPFHANDLER, "letting handler at %p attempt GPF recovery", eip);
            stack.eip = eip;
        } else {
            PANICFORWARD( GPF_DESCRIPTION, gpr, stack );
        }
    }
    else {
        APP_PANIC(GPF_DESCRIPTION, gpr, stack);
    }
}
