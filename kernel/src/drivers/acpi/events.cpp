/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <kernel/log/log.h>
#include <kernel/drivers/acpi/events.h>
#include <kernel/synch/eventfs.h>
#include <kernel/drivers/acpi/acpica/acpica.h>

static void acpi_notify_handler(ACPI_HANDLE handle, UINT32 value, void* context) {
    TAG_INFO(ACPICA, "acpi_notify_handler(0x%p, %u, 0x%p)", handle, value, context);
}

static void acpi_event_handler(UINT32 type, ACPI_HANDLE device, UINT32 number, void* context) {
    TAG_INFO(ACPICA, "acpi_event_handler(%u, 0x%p, %u, 0x%p)", type, device, number, context);
}

void AcpiEvents::onPowerButtonPress() {
    mPowerButtonFile->ioctl(IOCTL_EVENT_RAISE, 1);
    TAG_INFO(ACPICA, "power button press intercepted");
}
static UINT32 acpi_power_button_event_handler(void*) {
    AcpiEvents::get().onPowerButtonPress();
    return ACPI_INTERRUPT_HANDLED;
}

#define IS_ERR(x) ((x) != AE_OK)

uint32_t AcpiEvents::installEventHandlers() {
    auto result = AcpiInstallNotifyHandler(ACPI_ROOT_OBJECT, ACPI_SYSTEM_NOTIFY, acpi_notify_handler, nullptr);
    if (IS_ERR(result)) return result;

    result = AcpiInstallGlobalEventHandler(acpi_event_handler, nullptr);
    if (IS_ERR(result)) return result;

    result = AcpiInstallFixedEventHandler(ACPI_EVENT_POWER_BUTTON, acpi_power_button_event_handler, nullptr);
    return result;
}

AcpiEvents::AcpiEvents() {
    mPowerButtonFile = EventFS::get()->open("/acpi_power_button", FILE_OPEN_READ | FILE_OPEN_WRITE);
}

AcpiEvents& AcpiEvents::get() {
    static AcpiEvents gEvents;

    return gEvents;
}

