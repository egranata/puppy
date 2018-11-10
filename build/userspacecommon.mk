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

SHELL=/bin/bash

mklist_count := $(words "foo $(MAKEFILE_LIST)")
mklist_last := $(shell echo $$(( $(words $(MAKEFILE_LIST)) - 1 )))
mkfile_name := $(word $(mklist_last),$(MAKEFILE_LIST))
mkfile_path := $(abspath $(mkfile_name))
current_dir := $(patsubst %/,%,$(dir $(mkfile_path)))
app_path = $(shell basename $(current_dir))
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
find_c_files = $(call rwildcard,$1,*.c)
find_cpp_files = $(call rwildcard,$1,*.cpp)

CC=$(PUPPY_ROOT)/build/gcc.sh
CXX=$(PUPPY_ROOT)/build/gcc.sh

