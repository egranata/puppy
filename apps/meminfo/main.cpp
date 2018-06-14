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

#include <libuserspace/memory.h>
#include <libuserspace/printf.h>

int main(int, const char**) {
    auto mi(meminfo());

    printf("Total System Memory: %u bytes\nFree System Memory:  %u bytes\n",
        mi.total, mi.free);

    return 0;
}
