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

#include "../include/commands.h"
#include <newlib/stdio.h>

Commands::Commands() {
    addCommand("?", [this] (Document&) -> bool {
        printf("Available commands: \n");
        for (const auto& cmd : mCommands) {
            printf("%s\n", cmd.first.c_str());
        }
        return true;
    });
}

void Commands::addCommand(const eastl::string& name, CmdHandler f) {
    mCommands.emplace(name, f);
}
bool Commands::handleCommand(const eastl::string& command, Document& doc) {
    auto iter = mCommands.find(command);
    if (iter == mCommands.end()) {
        return false;
    } else {
        return iter->second(doc);
    }
}
