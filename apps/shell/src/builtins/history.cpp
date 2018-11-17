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

#include "../../include/builtin.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <linenoise/linenoise.h>

bool history_exec(size_t argc, char**) {
    if (argc == 1) {
        int history_size = linenoiseHistoryCopy(nullptr, 0);
        char** history_array = (char**)calloc(history_size, sizeof(char*));
        linenoiseHistoryCopy(history_array, history_size);
        for(int i = 0; i < history_size; ++i) {
            printf("%d: %s\n", i+1, history_array[i]);
            free(history_array[i]);
        }
        free(history_array);
    }

    return true;
}
REGISTER_BUILTIN(history, history_exec);
