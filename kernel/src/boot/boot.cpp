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

namespace boot {
    namespace config {
        uint32_t init();
    }
    namespace logging {
        uint32_t init();
    }
    namespace info {
        uint32_t init();
    }
    namespace time {
        uint32_t init();
    }
    namespace pic {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace pit {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace irq {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace i386 {
        uint32_t init();
    }
    namespace smbios {
        uint32_t init();
    }
    namespace acpica {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace apic {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace pmtimer {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace klog {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace task {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace vfs {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace ramdisk {
        uint32_t init();
    }
    namespace prng {
        uint32_t init();
    }
    namespace ramdevice {
        uint32_t init();
    }
    namespace pci {
        uint32_t init();
    }
    namespace mount {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace syscalls {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace irqcount {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace time_files {
        uint32_t init();
    }
    namespace msg_queue {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace semaphorefs {
        uint32_t init();
        bool fail(uint32_t);
    }
    namespace mutexfs {
        uint32_t init();
        bool fail(uint32_t);
    }
}

__attribute__((constructor)) void loadBootPhases() {
    registerBootPhase(bootphase_t{
        description : "Parse Kernel Commandline",
        visible : false,
        operation : boot::config::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "Configure logging",
        visible : false,
        operation : boot::logging::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "Parse Time Manager",
        visible : false,
        operation : boot::time::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "VFS",
        visible : false,
        operation : boot::vfs::init,
        onSuccess : nullptr,
        onFailure : boot::vfs::fail
    });

    registerBootPhase(bootphase_t{
        description : "Setup Interrupt Controller",
        visible : false,
        operation : boot::pic::init,
        onSuccess : nullptr,
        onFailure : boot::pic::fail
    });

    registerBootPhase(bootphase_t{
        description : "Setup Legacy PIT",
        visible : false,
        operation : boot::pit::init,
        onSuccess : nullptr,
        onFailure : boot::pit::fail
    });

    registerBootPhase(bootphase_t{
        description : "Enable IRQs",
        visible : false,
        operation : boot::irq::init,
        onSuccess : nullptr,
        onFailure : boot::irq::fail
    });

    registerBootPhase(bootphase_t{
        description : "ACPICA initialization",
        visible : false,
        operation : boot::acpica::init,
        onSuccess : nullptr,
        onFailure : boot::acpica::fail
    });

    registerBootPhase(bootphase_t{
        description : "Print System Info",
        visible : false,
        operation : boot::info::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "Setup CPU state",
        visible : false,
        operation : boot::i386::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "APIC discovery",
        visible : false,
        operation : boot::apic::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "PM Timer discovery",
        visible : false,
        operation : boot::pmtimer::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "PCI",
        visible : false,
        operation : boot::pci::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "Volume discovery",
        visible : false,
        operation : boot::mount::init,
        onSuccess : nullptr,
        onFailure : boot::mount::fail
    });

    registerBootPhase(bootphase_t{
        description : "Prepare system calls handlers",
        visible : false,
        operation : boot::syscalls::init,
        onSuccess : nullptr,
        onFailure : boot::syscalls::fail
    });

    registerBootPhase(bootphase_t{
        description : "Forward interrupt counters to userspace",
        visible : false,
        operation : boot::irqcount::init,
        onSuccess : nullptr,
        onFailure : boot::irqcount::fail
    });

    registerBootPhase(bootphase_t{
        description : "Forward date/time to userspace",
        visible : false,
        operation : boot::time_files::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "Install RAMDisk driver",
        visible : false,
        operation : boot::ramdisk::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "Install PRNG driver",
        visible : false,
        operation : boot::prng::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "Install RAM driver",
        visible : false,
        operation : boot::ramdevice::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "SMBIOS discovery",
        visible : false,
        operation : boot::smbios::init,
        onSuccess : nullptr,
        onFailure : nullptr
    });

    registerBootPhase(bootphase_t{
        description : "Prepare message queues support",
        visible : false,
        operation : boot::msg_queue::init,
        onSuccess : nullptr,
        onFailure : boot::msg_queue::fail
    });

    registerBootPhase(bootphase_t{
        description : "Prepare semaphores support",
        visible : false,
        operation : boot::semaphorefs::init,
        onSuccess : nullptr,
        onFailure : boot::semaphorefs::fail
    });

    registerBootPhase(bootphase_t{
        description : "Prepare mutex support",
        visible : false,
        operation : boot::mutexfs::init,
        onSuccess : nullptr,
        onFailure : boot::mutexfs::fail
    });

    registerBootPhase(bootphase_t{
        description : "Register Kernel Log Driver",
        visible : false,
        operation : boot::klog::init,
        onSuccess : nullptr,
        onFailure : boot::klog::fail
    });

    registerBootPhase(bootphase_t{
        description : "Enter multitasking",
        visible : false,
        operation : boot::task::init,
        onSuccess : nullptr,
        onFailure : boot::task::fail
    });
}
