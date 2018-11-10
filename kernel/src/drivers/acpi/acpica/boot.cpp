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

        return bootphase_t::gSuccess;
    }

    bool fail(uint32_t code) {
        if (code == gFatalFailure) return bootphase_t::gPanic;
        return bootphase_t::gContinueBoot;
    }
}
