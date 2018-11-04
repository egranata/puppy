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

#include "../include/getline.h"
#include <linenoise/linenoise.h>

#define HISTORY_FILE "/system/config/pupped.history"

eastl::string getline(bool history, const char* prompt) {
    static bool didLoadHistory = false;
    if (!didLoadHistory) linenoiseHistoryLoad(HISTORY_FILE);
    if (prompt == nullptr) prompt = "> ";
    char* data = linenoise(prompt);
    auto out = eastl::string(data);
    if (history) {
        linenoiseHistoryAdd(data);
        linenoiseHistorySave(HISTORY_FILE);
    }
    linenoiseFree(data);
    if (!out.empty() && out[out.size() - 1] == '\n') {
        out.pop_back();
    }
    return out;
}
