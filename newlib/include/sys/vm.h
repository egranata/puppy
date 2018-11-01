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

#ifndef NEWLIB_VM
#define NEWLIB_VM

#include <newlib/impl/cenv.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    VM_REGION_READONLY = 0,
    VM_REGION_READWRITE = 1
};

void* mapregion(size_t, int);
int protect(void*, int);
int unmapregion(void*);

int readable(void* p, size_t);
int writable(void* p, size_t);

#ifdef __cplusplus
}
#endif

#endif
