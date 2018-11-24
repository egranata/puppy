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
#include <shared_ptr>
#include <optional>
#include "symbol.h"

std::string yylval;
int yylex();
extern const char* yytext;

extern "C" int yywrap() { return 1; }

int yyinput() {
    return fgetc(stdin);
}

class Symbol {
    public:
        Symbol() : Symbol(INVALID, nullptr) {}
        Symbol(symbol_kind_t k, const char* v) : mKind(k), mValue(v ? v : "") {}

        static std::optional<Symbol> fromLexer() {
            auto k = yylex();
            if (k == 0) return {};
            return Symbol((symbol_kind_t)k, yylval.c_str());
        }

        symbol_kind_t kind() const { return mKind; }
        bool accept(symbol_kind_t k) const { return mKind == k; }
        const char* value() const { return mValue.c_str(); }
    private:
        symbol_kind_t mKind;
        std::string mValue;
};

class Redirect {
    public:
        Redirect(const char* target) : mTarget(target ? target : mTarget) {}
        const char* target() const { return mTarget.c_str(); }
    private:
        std::string mTarget;
};

class Command {
    public:
        Command() = default;
        void addWord(const char* word) {
            mWords.push_back(word);
        }
        void setRedirect(const char* target) {
            mRedirect = Redirect(target);
        }
        void setPipe(const Command& cmd) {
            mPipeTarget.reset(new Command(cmd));
        }
        void clear() {
            mWords.clear();
            mRedirect.reset();
            mPipeTarget.reset();
        }
        const auto& pipe() const { return mPipeTarget; }
        const auto begin() const { return mWords.begin(); }
        const auto end() const { return mWords.end(); }
        const auto& redirect() const { return mRedirect; }

        void printf() const {
            for (const auto& word : mWords) {
                ::printf("%s ", word.c_str());
            }
            if (mRedirect) {
                ::printf(" > %s", mRedirect->target());
            }
            if (mPipeTarget) {
                ::printf(" | ");
                mPipeTarget->printf();
            }
        }
    private:
        std::vector<std::string> mWords;
        std::optional<Redirect> mRedirect;
        std::shared_ptr<Command> mPipeTarget;
};

class Parser {
    public:
        Parser() = default;

        bool parse() {
            mCurrentCommand.clear();
            if (!nextsym()) return false;
            return parseCommand(mCurrentCommand);
        }
        const Command& command() const { return mCurrentCommand; }
    private:
        bool acceptText(std::string& dest) {
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
                dest = getenv(dest.c_str());
                return true;
            }

            return false;
        }
        bool parseCommand(Command& dest) {
            while(true) {
                std::string word;
                if (accept(TERMINATOR)) break;

                if (acceptText(word)) dest.addWord(word.c_str());

                if (accept(REDIRECT)) {
                    if (!nextsym()) {
                        error("redirect requires a valid file path");
                        return false;
                    }
                    if (acceptText(word)) dest.setRedirect(word.c_str());
                    if (acceptNext(TERMINATOR)) return true;
                    else {
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

                if (!nextsym()) break;
            }
            return true;
        }

        void error(const char* msg) {
            fprintf(stderr, "error: %s\n", msg);
        }
        bool nextsym() {
            if (auto sym = Symbol::fromLexer()) {
                mSymbol = *sym;
                return true;
            } return false;
        }
        bool accept(symbol_kind_t sk) {
            return cursym().accept(sk);
        }
        bool acceptNext(symbol_kind_t sk) {
            if (nextsym()) return accept(sk);
            return false;
        }
        const Symbol& cursym() { return mSymbol; }
        Command mCurrentCommand;
        Symbol mSymbol;
};

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
