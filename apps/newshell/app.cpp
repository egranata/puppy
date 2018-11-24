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

#include <stdio.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <memory>
#include <optional>
#include "symbol.h"
#include "parser.h"
#include "command.h"

std::string yylval;
extern const char* yytext;

extern "C" int yywrap() { return 1; }

int yyinput() {
    return fgetc(stdin);
}

int main()
{
    Parser parser;
    while (parser.parse()) {
        printf("> parsed command: ");
        parser.command().printf();
        printf("\n");
    }
    return 0;
}
