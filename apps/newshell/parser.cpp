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

#include "parser.h"

Parser::Parser() = default;

bool Parser::parse() {
    mCurrentCommand.clear();
    if (!nextsym()) return false;
    return parseCommand(mCurrentCommand);
}

const Command& Parser::command() const { return mCurrentCommand; }

bool Parser::acceptText(std::string& dest) {
    if (accept(ARGUMENT)) {
        dest = cursym().value();
        return true;
    }

    if (accept(QUOTED)) {
        dest = cursym().value();
        dest.erase(dest.begin());
        dest.pop_back();
        return true;
    }

    if (accept(ENVIRONMENT)) {
        dest = cursym().value();
        dest.erase(0,2);
        dest.pop_back();
        auto envv = getenv(dest.c_str());
        if (envv) dest = envv;
        else dest = "";
        return true;
    }

    return false;
}

bool Parser::parseCommand(Command& dest) {
    while(true) {
        std::string word;
        if (accept(TERMINATOR)) break;

        if (acceptText(word)) dest.addWord(word.c_str());

        if (accept(REDIRECT)) {
            if (!nextsym()) {
                error("redirect requires a valid file path");
                return false;
            }
            if (acceptText(word)) {
                dest.setRedirect(word.c_str());
            } else {
                error("redirect needs a destination");
                return false;
            }
            if (acceptNext(TERMINATOR)) return true; else {
                error("cannot add more arguments after a redirect");
                return false;
            }
        }

        if (accept(PIPE)) {
            if (!nextsym()) {
                error("pipe requires a valid command");
                return false;
            }
            Command temp;
            if (!parseCommand(temp)) {
                error("pipe requires a valid command");
                return false;
            }
            dest.setPipe(temp);
            return true;
        }

        if (accept(BACKGROUND)) {
            dest.setBackground(true);
            if (acceptNext(TERMINATOR)) return true; else {
                error("cannot add more arguments after a background");
                return false;
            }
        }

        if (!nextsym()) break;
    }
    return true;
}

void Parser::error(const char* msg) {
    fprintf(stderr, "error: %s\n", msg);
}
bool Parser::nextsym() {
    if (auto sym = Symbol::fromLexer()) {
        mSymbol = *sym;
        return true;
    } return false;
}
bool Parser::accept(symbol_kind_t sk) {
    return cursym().accept(sk);
}
bool Parser::acceptNext(symbol_kind_t sk) {
    if (nextsym()) return accept(sk);
    return false;
}
const Symbol& Parser::cursym() { return mSymbol; }
