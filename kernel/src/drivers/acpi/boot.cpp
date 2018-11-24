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
#include <kernel/drivers/acpi/acpica/acpica.h>
#include <kernel/drivers/pic/pic.h>
#include <kernel/drivers/acpi/acpica/osi.h>
#include <kernel/drivers/acpi/device.h>

#define IS_ERR (acpi_init != AE_OK)

static void acpi_notify_handler(ACPI_HANDLE handle, UINT32 value, void* context) {
    TAG_INFO(ACPICA, "acpi_notify_handler(0x%p, %u, 0x%p)", handle, value, context);
}

static void acpi_event_handler(UINT32 type, ACPI_HANDLE device, UINT32 number, void* context) {
    TAG_INFO(ACPICA, "acpi_event_handler(%u, 0x%p, %u, 0x%p)", type, device, number, context);
}

static UINT32 acpi_power_button_event_handler(void* context) {
    TAG_INFO(ACPICA, "acpi_power_button_event_handler(0x%p)", context);
    return ACPI_INTERRUPT_HANDLED;
}

static void acpi_scan_callback(const AcpiDeviceManager::acpica_device_t& device, void* ctx) {
    uint64_t *count = (uint64_t*)ctx;
    ++*count;
    TAG_INFO(ACPICA, "device discovered: %s (hid=%s)", device.pathname,
        device.devinfo->HardwareId.String ? device.devinfo->HardwareId.String : "none");
}

namespace boot::acpica {
    constexpr uint32_t gFatalFailure = 1;
    constexpr uint32_t gNonFatalFailure = 2;

    uint32_t init() {
        AcpiGbl_DoNotUseXsdt = true;

        AcpiInstallInterfaceHandler(OsiHandler);
        AddOsiInterface("Windows 2009");
        EnableOsi();

        auto acpi_init = AcpiInitializeSubsystem();
        if (IS_ERR) return gNonFatalFailure;

        acpi_init = AcpiInitializeTables(nullptr, 32, false);
        if (IS_ERR) return gNonFatalFailure;

        acpi_init = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_MEMORY, ACPI_DEFAULT_HANDLER, nullptr, nullptr);
        if (IS_ERR) return gNonFatalFailure;

        acpi_init = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_IO, ACPI_DEFAULT_HANDLER, nullptr, nullptr);
        if (IS_ERR) return gNonFatalFailure;

        acpi_init = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_PCI_CONFIG, ACPI_DEFAULT_HANDLER, nullptr, nullptr);
        if (IS_ERR) return gNonFatalFailure;

        acpi_init = AcpiLoadTables();
        if (IS_ERR) return gNonFatalFailure;

        acpi_init = AcpiInstallNotifyHandler(ACPI_ROOT_OBJECT, ACPI_SYSTEM_NOTIFY, acpi_notify_handler, nullptr);
        if (IS_ERR) return gNonFatalFailure;

        acpi_init = AcpiEnableSubsystem(0);
        if (IS_ERR) return gNonFatalFailure;

        // from here on out, we have an ACPI IRQ installed, so any failure would leave
        // the system in an inconsistent state and is hence fatal
        acpi_init = AcpiInitializeObjects(0);
        if (IS_ERR) return gFatalFailure;

        acpi_init = PIC::get().setupACPI();
        if (IS_ERR) return gFatalFailure;

        acpi_init = AcpiInstallGlobalEventHandler(acpi_event_handler, nullptr);
        if (IS_ERR) return gFatalFailure;

        acpi_init = AcpiInstallFixedEventHandler(ACPI_EVENT_POWER_BUTTON, acpi_power_button_event_handler, nullptr);
        if (IS_ERR) return gFatalFailure;

        acpi_init = AcpiEnableEvent(ACPI_EVENT_POWER_BUTTON, 0);
        if (IS_ERR) return gFatalFailure;

        AcpiDeviceManager& acpi_dev_mgr(AcpiDeviceManager::get());
        uint64_t num_acpi_devs = 0;
        acpi_init = acpi_dev_mgr.discoverDevices(acpi_scan_callback, &num_acpi_devs);
        if (IS_ERR) return gFatalFailure;

        bootphase_t::printf("%llu ACPI devices detected\n", num_acpi_devs);

        acpi_dev_mgr.tryLoadDrivers();

        DevFS& devfs(DevFS::get());
        auto acpiDir = devfs.getDeviceDirectory("acpi");
        acpi_dev_mgr.exportToDevFs(acpiDir);

        return bootphase_t::gSuccess;
    }

    bool fail(uint32_t code) {
        if (code == gFatalFailure) return bootphase_t::gPanic;
        return bootphase_t::gContinueBoot;
    }
}
