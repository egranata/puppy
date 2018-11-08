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

#include <kernel/drivers/acpi/acpica/irq.h>
#include <kernel/drivers/pic/pic.h>
#include <kernel/i386/idt.h>

extern "C" ACPI_STATUS AcpiOsInstallInterruptHandler (UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine, void *Context) {
    auto irq = PIC::gIRQNumber(InterruptNumber);
    Interrupts::get().sethandler(irq, "ACPI", (Interrupts::handler_t::irq_handler_f)ServiceRoutine, Context);
    PIC::get().accept(InterruptNumber);
    TAG_DEBUG(ACPICA, "AcpiOsInstallInterruptHandler(%u, %p, %p)", InterruptNumber, ServiceRoutine, Context);
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsRemoveInterruptHandler (UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine) {
    TAG_DEBUG(ACPICA, "AcpiOsRemoveInterruptHandler(%u, %p)", InterruptNumber, ServiceRoutine);
    return AE_OK;
}
