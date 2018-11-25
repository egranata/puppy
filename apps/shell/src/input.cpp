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

#include "input.h"
#include <stdio.h>

const char* yylval;
extern "C" int yywrap() { return 1; }
extern "C" void* yy_scan_string (const char*);
extern "C" void yy_delete_buffer (void*);

void setInputBuffer(const char* buffer) {
    static void* gCurrentBuffer = nullptr;
    if (gCurrentBuffer) {
        yy_delete_buffer(gCurrentBuffer);
    }
    std::string newBuffer; newBuffer.append_sprintf("%s;", buffer);
    gCurrentBuffer = yy_scan_string(newBuffer.c_str());
    if (gCurrentBuffer == nullptr) {
        fprintf(stderr, "error in setting input buffer\n");
    }
}
