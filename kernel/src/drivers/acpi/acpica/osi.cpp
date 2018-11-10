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

#include <kernel/drivers/acpi/acpica/osi.h>
#include <kernel/libc/vec.h>

extern "C" UINT32 OsiHandler(ACPI_STRING interface, UINT32 supported) {
    TAG_INFO(ACPICA, "OsiHandler(%s, %u)", interface, supported);
    return supported;
}

static vector<const char*>& gACPIInterfaces() {
    static vector<const char*> gVector;
    // add a few default OSI strings (courtesy of https://github.com/Meulengracht/MollenOS/blob/af01ec684f1aadabf859b7797e4b2ec3dc7b9272/kernel/acpi/osl.c)
    if (gVector.empty()) {
        gVector.push_back("Module Device");
        gVector.push_back("Processor Device");
        gVector.push_back("3.0 _SCP Extensions");
        gVector.push_back("Processor Aggregator Device");
    }
    return gVector;
}

void AddOsiInterface(const char* interface) {
    if (interface && *interface) gACPIInterfaces().push_back(interface);
}

void EnableOsi() {
    for (const auto& ifce : gACPIInterfaces()) {
        AcpiInstallInterface((ACPI_STRING)ifce);
    }
}
