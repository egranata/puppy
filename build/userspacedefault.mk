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

c_files := $(call find_c_files,$(current_dir))
cpp_files := $(call find_cpp_files,$(current_dir))
objects := $(patsubst %.c,%.o,$(cpp_files))
objects += $(patsubst %.c,%.o,$(c_files))

all : $(objects)
	@echo "Building $(app_path)..."
	$(CC) -o $(PUPPY_ROOT)/$(OUTWHERE)/$(app_path) $(objects)

