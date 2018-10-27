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

#include "../include/builtin.h"
#include <stdint.h>
#include <newlib/stdlib.h>
#include <newlib/string.h>
#include <EASTL/string.h>
#include <EASTL/unordered_map.h>

namespace {
    using builtin_map = eastl::unordered_map<eastl::string, builtin_cmd_f>;
    builtin_map& getBuiltinMap() {
        static builtin_map gMap;
        return gMap;
    }
}

bool registerBuiltinCommand(const char* name, builtin_cmd_f f) {
    auto& map = getBuiltinMap();
    auto ok = map.emplace(name, f);
    return ok.second;
}

bool tryExecBuiltin(const char* program, size_t argc, char** args) {
    // ignore empty lines and comments
    if (program == nullptr || program[0] == 0) return true;
    if (program[0] == '#') return true;

    auto iter = getBuiltinMap().find(program), end = getBuiltinMap().end();
    if (iter == end) return false;
    return iter->second(argc, args);
}
