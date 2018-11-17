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

#include <unordered_map>
#include <functional>
#include <string>

class Document;

class Commands {
    public:
        using CmdHandler = std::function<bool(Document&)>;

        Commands();

        void addCommand(const std::string& name, CmdHandler f);
        bool handleCommand(const std::string& command, Document& doc);

    private:
        std::unordered_map<std::string, CmdHandler> mCommands;
};

#endif
