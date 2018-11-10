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

#define IS_ERR (acpi_init != AE_OK)

static void acpi_notify_handler(ACPI_HANDLE handle, UINT32 value, void* context) {
    TAG_INFO(ACPICA, "acpi_notify_handler(%p, %u, %p)", handle, value, context);
}

static void acpi_event_handler(UINT32 type, ACPI_HANDLE device, UINT32 number, void* context) {
    TAG_INFO(ACPICA, "acpi_event_handler(%u, %p, %u, %p)", type, device, number, context);
}

static UINT32 acpi_power_button_event_handler(void* context) {
    TAG_INFO(ACPICA, "acpi_power_button_event_handler(%p)", context);
    return ACPI_INTERRUPT_HANDLED;
}

static ACPI_STATUS acpi_device_scanner(ACPI_HANDLE handle, UINT32 level, void*, void**) {
    ACPI_OBJECT_TYPE type;

    ACPI_STATUS ok = AcpiGetType(handle, &type);
    if (ok != AE_OK) {
        TAG_ERROR(ACPICA, "ACPI enumeration failed, handle: %p level: %u", handle, level);
        return AE_OK;
    }
    ACPI_BUFFER nameBuffer = {0};
    nameBuffer.Length = 128;
    nameBuffer.Pointer = calloc(sizeof(char), nameBuffer.Length);
    ok = AcpiGetName(handle, ACPI_FULL_PATHNAME, &nameBuffer);
    ACPI_BUFFER diBuffer = {0};
    diBuffer.Length = sizeof(ACPI_DEVICE_INFO);
    diBuffer.Pointer = calloc(1, sizeof(ACPI_DEVICE_INFO));
    ACPI_DEVICE_INFO *di = (ACPI_DEVICE_INFO*)diBuffer.Pointer;
    ok = AcpiGetObjectInfo(handle, &di);
    if (ok == AE_OK && (di->Valid & ACPI_VALID_HID)) {
        TAG_INFO(ACPICA, "device detect: path=%s hid=%s", nameBuffer.Pointer, di->HardwareId.String);
    } else {
        TAG_INFO(ACPICA, "device detect: path=%s hid=<invalid>", nameBuffer.Pointer);
    }
    // TODO: do something useful with the devices
    free(nameBuffer.Pointer);
    free(diBuffer.Pointer);
    return AE_OK;
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

        acpi_init = AcpiGetDevices(nullptr, acpi_device_scanner, nullptr, nullptr);
        if (IS_ERR) return gFatalFailure;

        return bootphase_t::gSuccess;
    }

    bool fail(uint32_t code) {
        if (code == gFatalFailure) return bootphase_t::gPanic;
        return bootphase_t::gContinueBoot;
    }
}
