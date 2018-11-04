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

#include <kernel/boot/phase.h>
#include <kernel/sys/osinfo.h>
#include <kernel/i386/cpuid.h>
#include <kernel/drivers/framebuffer/fb.h>

namespace boot::info {
    uint32_t init() {
        bootphase_t::printf("%s built on %s at %s\n", OSNAME, __DATE__, __TIME__);

    	auto&& cpuid(CPUID::get());
        auto&& sig(cpuid.getSignature());
        auto&& features(cpuid.getFeatures());
        bootphase_t::printf("CPU: %s %s\n", cpuid.getVendorString(), cpuid.getBrandString());
        bootphase_t::printf("     Family %u Model %u Stepping %u\n", sig.family, sig.model, sig.stepping);
        bootphase_t::printf("     Supported Features: ");
        #define CPU_FEATURE(name, reg, bit) {\
            if (features. name) { bootphase_t::printf("%s ", #name); } \
        }
      	#include <kernel/i386/features.tbl>
    	#undef CPU_FEATURE
        bootphase_t::printf("\n");

        auto&& fb(Framebuffer::get());

        bootphase_t::printf("Screen size: %u x %u (%u rows x %u columns)\n",
            fb.width(), fb.height(),
            fb.rows(), fb.columns());

        return 0;
    }
}
