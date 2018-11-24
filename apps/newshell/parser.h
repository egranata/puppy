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

#ifndef NEWSHELL_PARSER
#define NEWSHELL_PARSER

#include <string>
#include "symbol.h"
#include "command.h"

class Parser {
    public:
        Parser();

        bool parse();
        const Command& command() const;
    private:
        // accept an argument or anything that we know how to expand
        // into an argument
        bool acceptText(std::string& dest);
        bool parseCommand(Command& dest);

        void error(const char* msg);

        bool nextsym();
        bool accept(symbol_kind_t sk);
        bool acceptNext(symbol_kind_t sk);
        const Symbol& cursym();

        Command mCurrentCommand;
        Symbol mSymbol;
};

#endif
