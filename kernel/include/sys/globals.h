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

#ifndef SYS_GLOBALS
#define SYS_GLOBALS

#include <kernel/sys/sanity.h>
#include <kernel/sys/stdint.h>

#define GLOBAL_ITEM(varname, apiname) extern uintptr_t varname; \
template<typename T = uintptr_t> \
static T addr_ ## apiname () { \
	return (T)& varname; \
} \
template<typename T = uintptr_t> \
static T val_ ## apiname () { \
	return *(addr_ ## apiname<T*>()); \
} \

GLOBAL_ITEM(__kernel_start, 			 kernel_start);
GLOBAL_ITEM(__ctors_start,  		     ctors_start);
GLOBAL_ITEM(__ctors_end,    			 ctors_end);
GLOBAL_ITEM(__acpi_ddriv_start,  		 acpi_devices_start);
GLOBAL_ITEM(__acpi_ddriv_end,  		     acpi_devices_end);
GLOBAL_ITEM(__kernel_end,   			 kernel_end);
GLOBAL_ITEM(__bootpagetbl,  			 bootpagetable);
GLOBAL_ITEM(__bootpagedir,  		     bootpagedirectory);
GLOBAL_ITEM(__gdt,          			 gdt);
GLOBAL_ITEM(__numsysgdtentries,          numsysgdtentries);

#endif
