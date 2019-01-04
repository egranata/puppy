%{
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

#define YYSTYPE const char*
#include <string.h>
#include <stdio.h>
#include "symtype.h"
extern const char* yylval;
%}
%%
$\([,0-9A-Za-z:/\-\_=\.~]+\) yylval = yytext; return ENVIRONMENT;
[,0-9A-Za-z:/\-\_=\.~]+ yylval = yytext; return ARGUMENT;
[>]   return OUTREDIR;
[<]   return INREDIR;
[|]   return PIPE;
[ \t]+ /* ignore whitespace */;
[;\n]   return TERMINATOR;
&&    return SUCCESSCONCAT;
&     return BACKGROUND;
!!    return HISTORY;
\"(\\.|[^\\"])*\" yylval = yytext; return QUOTED;
%%
