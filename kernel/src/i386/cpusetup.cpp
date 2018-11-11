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

#include <kernel/log/log.h>

#include <kernel/i386/primitives.h>
#include <kernel/i386/cpuid.h>
#include <kernel/boot/phase.h>
#include <kernel/drivers/cpu/device.h>
#include <kernel/i386/mtrr.h>
#include <kernel/drivers/framebuffer/fb.h>

namespace boot::i386 {
    uint32_t init() {
        // must clear this bit
        static constexpr uint32_t rdpcm_ring3_allow = 0x00000100;
        // must set this bit
        static constexpr uint32_t rdtsc_ring3_prevent = 0x4;

        static constexpr uint32_t no_x87_emu = ~(1 << 2);
        static constexpr uint32_t fwait_ts = 1 << 1;

        static constexpr uint32_t osfxsr = 1 << 9;
        static constexpr uint32_t unmasked_smid_except = 1 << 10;

        // prevent userspace from accessing detailed timing information
        auto original_cr4 = readcr4();
        auto cr4 = original_cr4 | rdtsc_ring3_prevent;
        cr4 &= ~rdpcm_ring3_allow;
        writecr4(cr4);
        cr4 = readcr4();

        auto& cpuid(CPUID::get());
        if (!cpuid.getFeatures().sse) {
            bootphase_t::printf("No SSE support detected; leaving disabled\n");
            fpsave = fpsave_fpu;
            fprestore = fprestore_fpu;
        } else {
            // enable SSE floating point state
            auto original_cr0 = readcr0();
            auto cr0 = original_cr0 & no_x87_emu;
            cr0 |= fwait_ts;
            writecr0(cr0);
            cr4 |= osfxsr;
            cr4 |= unmasked_smid_except;
            writecr4(cr4);
            fpsave = fpsave_sse;
            fprestore = fprestore_sse;
            LOG_DEBUG("original CR0 = 0x%p, final CR0 = 0x%p", original_cr0, cr0);
        }

        LOG_DEBUG("fpsave = 0x%p, fprestore = 0x%p [SSE variant is 0x%p,0x%p; FPU variant is 0x%p,0x%p]",
            fpsave, fprestore,
            fpsave_sse, fprestore_sse,
            fpsave_fpu, fprestore_fpu);

        LOG_DEBUG("original CR4 = 0x%p, final CR4 = 0x%p", original_cr4, cr4);

        CPUDevice::get();

        MTRR *mtrr = MTRR::tryGet();
        if (mtrr == nullptr) {
            LOG_WARNING("MTRR not supported by CPU");
        } else {
            LOG_INFO("MTRR support available");
            if (mtrr->doesWriteCombining()) {
                bool fbwc = Framebuffer::get().enableWriteCombining(mtrr);
                if (fbwc) {
                    LOG_INFO("Framebuffer will perform write combining");
                } else {
                    LOG_WARNING("Framebuffer can't write-combine; performance may suffer");
                }
            }
        }

        return 0;
    }
}
