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

all: helper test

helper:
	$(CC) helf.cpp -Xlinker -Ttext -Xlinker 0xC0100000 -o $(PUPPY_ROOT)/$(OUTWHERE)/helf

test: test.cpp
	$(CC) -DTEST_NAME=\"$(TGTNAME)\" -I$(PWD)/include test.cpp -lcheckup -o $(PUPPY_ROOT)/$(OUTWHERE)/$(TGTNAME)
