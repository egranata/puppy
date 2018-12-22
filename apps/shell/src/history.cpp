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
#include <unique_ptr>

History::History(const char* path) : mFilePath(path ? path : gDefaultHistoryFile) {
    linenoiseHistoryLoad(mFilePath.c_str());
    auto ml = linenoiseHistoryGetMaxLen();
    size_t i = 0;
    std::unique_ptr<char*> buffer;
    buffer.reset((char**)calloc(ml, sizeof(char**)));
    linenoiseHistoryCopy(buffer.get(), ml);
    for(char** ptr = buffer.get(); i < ml; ++i) {
        if (ptr[i] == nullptr) break;
        if (ptr[i][0]) mHistory.push_back(std::string(ptr[i]));
    }
}

History::~History() {
    save();
}

void History::add(const std::string& s) {
    linenoiseHistoryAdd(s.c_str());
    mHistory.push_back(s);
}

bool History::get(size_t idx, std::string& s) const {
    if (idx >= mHistory.size()) return false;

    s = mHistory[idx];
    return true;
}

size_t History::size() const {
    return mHistory.size();
}

void History::save() {
    linenoiseHistorySave(mFilePath.c_str());
}

History& History::defaultHistory() {
    static History gHistory;
    return gHistory;
}
