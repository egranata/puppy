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

#ifndef PUPPED_COMMANDS
#define PUPPED_COMMANDS

#include <EASTL/unordered_map.h>
#include <EASTL/functional.h>
#include <EASTL/string.h>

class Document;

class Commands {
    public:
        using CmdHandler = eastl::function<bool(Document&)>;

        Commands();

        void addCommand(const eastl::string& name, CmdHandler f);
        bool handleCommand(const eastl::string& command, Document& doc);

    private:
        eastl::unordered_map<eastl::string, CmdHandler> mCommands;
};

#endif
