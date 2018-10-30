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

#ifndef NEWLIB_COLLECT
#define NEWLIB_COLLECT

#include <newlib/impl/cenv.h>
#include <stdint.h>
#include <kernel/syscalls/types.h>

NEWLIB_IMPL_REQUIREMENT process_exit_status_t collect(uint16_t pid);

NEWLIB_IMPL_REQUIREMENT bool collectany(bool wait, uint16_t*, process_exit_status_t*);

#endif
