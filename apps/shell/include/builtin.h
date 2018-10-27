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

#ifndef SHELL_BUILTIN
#define SHELL_BUILTIN

#include <EASTL/functional.h>

typedef eastl::function<bool(size_t, char**)> builtin_cmd_f;

bool registerBuiltinCommand(const char*, builtin_cmd_f);

bool tryExecBuiltin(const char* program, size_t argc, char** args);

#define REGISTER_BUILTIN(name, executor) \
    static void __attribute__((constructor)) name ## _register() { \
        registerBuiltinCommand(#name, executor); \
    }

#endif
