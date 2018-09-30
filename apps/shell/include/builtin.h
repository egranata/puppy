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

struct builtin_cmd_t {
    const char* command;
    void(*executor)(const char*);
};

extern builtin_cmd_t builtin_cmds[];

bool tryExecBuiltin(const char* program, const char* args);

#endif