#!/usr/bin/python3
#
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

import sys

for i in sys.argv[1:]:
    n = int(i, base=0)
    b0 = n & 0xFF
    b1 = (n >> 8) & 0xFF
    b2 = (n >> 16) & 0xFF
    b3 = (n >> 24) & 0xFF

    print('%s --> "%s%s%s%s"' % (i, chr(b0), chr(b1), chr(b2), chr(b3)))
