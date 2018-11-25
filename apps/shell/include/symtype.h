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

#ifndef SHELL_SYMTYPE
#define SHELL_SYMTYPE

// leave it outside of Symbol because it makes the rest of the code more convenient
enum symbol_kind_t {
    INVALID = 0,
    ARGUMENT = 1,
    OUTREDIR = 2,
    INREDIR = 3,
    PIPE = 4,
    TERMINATOR = 5,
    ENVIRONMENT = 6,
    QUOTED = 7,
    BACKGROUND = 8,
};

#endif
