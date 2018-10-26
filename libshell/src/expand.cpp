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

#include <libshell/expand.h>
#include <EASTL/stack.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

namespace {
    enum class parser_state_t {
        ORDINARY,
        QUOTE,
        ESCAPE
    };
}

#define APPEND_NON_EMPTY { \
    if (!buffer.empty()) { \
        result.push_back(buffer); \
        buffer = ""; \
    } \
}

#define NEW_STATE(p) pstate.push(parser_state_t:: p)
#define BACK_STATE pstate.pop()
#define APPEND_TO_BUF buffer += c

namespace libShellSupport {
    char** parseCommandLine(const char* cmdline, size_t* argc) {
        eastl::vector<eastl::string> result;
        eastl::string buffer;
        eastl::stack<parser_state_t> pstate;
        NEW_STATE(ORDINARY);
        for(; cmdline && *cmdline; ++cmdline) {
            char c = *cmdline;
            parser_state_t state = pstate.top();
            switch (state) {
                case parser_state_t::ORDINARY:
                    if (c == '\\') {
                        NEW_STATE(ESCAPE);
                    } else if (c == '"') {
                        NEW_STATE(QUOTE);
                    } else if (c == ' ') {
                        APPEND_NON_EMPTY;
                    }
                    else {
                        APPEND_TO_BUF;
                    }
                    break;
                case parser_state_t::QUOTE:
                    if (c == '"') {
                        BACK_STATE;
                    } else if (c == '\\') {
                        NEW_STATE(ESCAPE);
                    } else {
                        APPEND_TO_BUF;
                    }
                    break;
                case parser_state_t::ESCAPE:
                    APPEND_TO_BUF;
                    BACK_STATE;
                    break;
            }
        }
        APPEND_NON_EMPTY;
        if (argc) *argc = result.size();
        char** cresult = (char**)calloc(1 + result.size(), sizeof(char*));
        for (size_t i = 0; i < result.size(); ++i) {
            const auto& in_str = result[i];
            cresult[i] = (char*)calloc(in_str.size() + 1, sizeof(char));
            strcpy(cresult[i], in_str.c_str());
        }
        return cresult;
    }

    // this is technically simple enough that one could put it outside of libShellSupport
    // but it is convenient to have it here along with the allocating API
    void freeCommandLine(char** argv) {
        size_t i = 0;
        while(argv[i]) {
            free(argv[i]);
            ++i;
        }
        free(argv);
    }
}

#undef APPEND_NON_EMPTY
#undef NEW_STATE
#undef BACK_STATE
#undef APPEND_TO_BUF
