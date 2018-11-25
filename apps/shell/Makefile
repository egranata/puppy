# Copyright 2018 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

INPUTS := $(wildcard src/*.cpp)
INPUTS += $(wildcard src/builtins/*.cpp)
INPUTS += main.cpp

all: $(INPUTS) src/lexer.l
	lex -o$(PUPPY_ROOT)/out/shell.lex.yy.c src/lexer.l
	$(CC) -I$(PWD)/include $(INPUTS) $(PUPPY_ROOT)/out/shell.lex.yy.c -o $(PUPPY_ROOT)/$(OUTWHERE)/shell
