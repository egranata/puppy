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

#ifndef NEWLIB_PROCESS
#define NEWLIB_PROCESS

#include <newlib/sys/signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPAWN_BACKGROUND 0
#define SPAWN_FOREGROUND 1

#define PROCESS_INHERITS_CWD 2
#define PROCESS_DEFAULT_CWD 0

#define PROCESS_INHERITS_ENVIRONMENT 0
#define PROCESS_EMPTY_ENVIRONMENT 4

int spawn(const char* path, const char* args, int flags);

#ifdef __cplusplus
}
#endif

#endif