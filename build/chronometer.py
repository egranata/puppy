#!/usr/bin/env python3
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

import time

class Chronometer(object):
    def __init__(self, phase):
        self.phase = phase
    
    def __enter__(self):
        self.begin = time.time()
        return self
    
    def __exit__(self, type, value, tb):
        self.end = time.time()
        duration = int(self.end - self.begin)
        if self.phase:
            if duration == 1:
                print("Build phase [%s]: complete in 1 second" % self.phase)
            elif duration > 0:
                print("Build phase [%s]: complete in %s seconds" % (self.phase, duration))
            else:
                print("Build phase [%s] : complete in under 1 second" % self.phase)

