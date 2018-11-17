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

#include "../include/str.h"
#include <stdlib.h>
#include <stdio.h>

void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(' '));
    s.erase(s.find_last_not_of(' ') + 1);
}

std::string getline(const char* prompt, bool& eof) {
    if (prompt == nullptr) prompt = "> ";
    printf("%s", prompt);
    char* data = nullptr;
    size_t len;
    size_t n_read = __getline(&data, &len, stdin);
    if (n_read == (size_t)-1 && feof(stdin)) {
        eof = true;
        return std::string();
    }
    eof = false;
    auto out = std::string(data);
    free(data);
    if (!out.empty() && out[out.size() - 1] == '\n') {
        out.pop_back();
    }
    return out;
}
