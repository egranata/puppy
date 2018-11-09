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

namespace {
    struct acpi_irq_t {
        ACPI_OSD_HANDLER acpi_handler_f;
        void* acpi_context;
        uint8_t pic_irq_n;
        uint8_t cpu_irq_n;
    };
    static void acpi_irq_f(GPR&, InterruptStack&, void* ctx) {
        acpi_irq_t *context = (acpi_irq_t*)ctx;

        context->acpi_handler_f(context->acpi_context);
        PIC::eoi(context->pic_irq_n);
    }
}

extern "C" ACPI_STATUS AcpiOsInstallInterruptHandler (UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine, void *Context) {
    acpi_irq_t *acpi_irq = (acpi_irq_t*)calloc(1, sizeof(acpi_irq_t));
    acpi_irq->pic_irq_n = InterruptNumber;
    acpi_irq->cpu_irq_n = PIC::gIRQNumber(InterruptNumber);
    acpi_irq->acpi_handler_f = ServiceRoutine;
    acpi_irq->acpi_context = Context;

    Interrupts::get().sethandler(acpi_irq->cpu_irq_n, "ACPI", acpi_irq_f, acpi_irq);
    PIC::get().accept(acpi_irq->pic_irq_n);

    TAG_DEBUG(ACPICA, "AcpiOsInstallInterruptHandler(%u, %p, %p)", InterruptNumber, ServiceRoutine, Context);
    return AE_OK;
}

extern "C" ACPI_STATUS AcpiOsRemoveInterruptHandler (UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine) {
    // TODO: remove the IRQ and free the associated context data
    TAG_DEBUG(ACPICA, "AcpiOsRemoveInterruptHandler(%u, %p)", InterruptNumber, ServiceRoutine);
    return AE_OK;
}
