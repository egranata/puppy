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
#include <newlib/stdio.h>

eastl::string getline(const char* prompt) {
    if (prompt == nullptr) prompt = "> ";
    printf("%s", prompt); fflush(stdout);
    char* data = nullptr;
    size_t len;
    __getline(&data, &len, stdin);
    auto out = eastl::string(data);
    free(data);
    if (!out.empty() && out[out.size() - 1] == '\n') {
        out.pop_back();
    }
    return out;
}
