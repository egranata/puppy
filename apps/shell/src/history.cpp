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

#include "../include/history.h"
#include <linenoise/linenoise.h>

History::History(const char* path) : mFilePath(path) {
    linenoiseHistoryLoad(mFilePath.c_str());
}

History::~History() {
    save();
}

void History::add(const eastl::string& s) {
    linenoiseHistoryAdd(s.c_str());
}

void History::save() {
    linenoiseHistorySave(mFilePath.c_str());
}

History& History::defaultHistory() {
    static History gHistory;
    return gHistory;
}