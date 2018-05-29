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

#include <drivers/acpi/rsdp.h>
#include <drivers/acpi/rsdt.h>
#include <drivers/acpi/fadt.h>

#include <mm/virt.h>

#include <i386/primitives.h>

#include <log/log.h>

extern "C"
void reboot8042();

extern "C"
void reboot() {
    LOG_DEBUG("asked to reboot");
    // TODO: only init should be allowed to do this; userspace needs to go down and by the time that's over
    // init (and the scheduler) would be the only processes left; the alternative might be for the kernel itself
    // to be the return point after userspace goes down - and do either this or shutdown
    auto rsdp = RSDP::tryget();
    if (nullptr == rsdp) { LOG_DEBUG("rebooting via 8042"); reboot8042(); }
    auto&& rsdt = rsdp->rsdt();
    auto fadt = rsdt.fadt();

    if (nullptr == fadt) { LOG_DEBUG("rebooting via 8042"); reboot8042(); }
    if (nullptr == fadt->fadt) { LOG_DEBUG("rebooting via 8042"); reboot8042(); }

    if (fadt->hdr.rev == 1) { LOG_DEBUG("rebooting via 8042"); reboot8042(); }

    auto&& resetreg = fadt->fadt->resetreg;
    auto resetval = fadt->fadt->resetvalue;

    LOG_DEBUG("ACPI reset register: space = %u, address = %llx, value = %u", resetreg.space, resetreg.address, resetval);

    switch (resetreg.space) {
        case acpi_address_t::addr_space_t::memory: {
            uint8_t *resetptr = (uint8_t*)resetreg.address;
            LOG_DEBUG("forcing ACPI reboot by writing %x to %p", resetval, resetptr);

            auto pg(VirtualPageManager::page((uintptr_t)resetptr));
            auto off(VirtualPageManager::offset((uintptr_t)resetptr));
            auto& vm(VirtualPageManager::get());
            vm.map(pg, 0x0, VirtualPageManager::map_options_t::kernel());

            LOG_DEBUG("mapped %p to page 0 - write will be to %p", resetptr, off);

            resetptr = (uint8_t*)off;
            *resetptr = resetval;
            break; // yeah, yeah, yeah..
        }
        case acpi_address_t::addr_space_t::io: {
            uint16_t resetport = (uint16_t)resetreg.address;
            LOG_DEBUG("forcing ACPI reboot by writing %x to io port %x", resetval, resetport);
            outb(resetport, resetval);
            break;
        }
        case acpi_address_t::addr_space_t::pci:
        case acpi_address_t::addr_space_t::ctrl:
        case acpi_address_t::addr_space_t::smbus:
        case acpi_address_t::addr_space_t::ffh: {
            LOG_WARNING("only memory and I/O ACPI reboot supported");
        }
    }

    LOG_WARNING("well that didn't reboot much... rebooting by keyboard controller");
    reboot8042();
}
