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

#ifndef SHELL_SYMBOL
#define SHELL_SYMBOL

#include <string>
#include <optional>
#include "symtype.h"

extern "C" int yylex();
extern const char *yylval;

class Symbol {
    public:
        Symbol() : Symbol(INVALID, nullptr) {}
        Symbol(symbol_kind_t k, const char* v) : mKind(k), mValue(v ? v : "") {}

        static std::optional<Symbol> fromLexer() {
            auto k = yylex();
            if (k == 0) return {};
            return Symbol((symbol_kind_t)k, yylval);
        }

        symbol_kind_t kind() const { return mKind; }
        bool accept(symbol_kind_t k) const { return mKind == k; }
        const char* value() const { return mValue.c_str(); }
    private:
        symbol_kind_t mKind;
        std::string mValue;
};

#endif
