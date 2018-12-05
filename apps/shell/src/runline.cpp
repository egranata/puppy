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

#include "runline.h"
#include "builtin.h"
#include "exit.h"
#include "path.h"
#include "str.h"
#include "input.h"
#include "parser.h"
#include "command.h"

#include <sys/wait.h>
#include <sys/process.h>

#include <libshell/expand.h>

bool runline(std::string cmdline) {
    trim(cmdline);
    if(cmdline.empty()) return true;
    if (cmdline.front() == '#') return true;

    setInputBuffer(cmdline.c_str());
    Parser parser;
    while (parser.parse()) {
        auto cmd = parser.command();
        if (cmd) {
            cmd.exec();
        }
    }

    // TODO: error handling
    return true;
}
