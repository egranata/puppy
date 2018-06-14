/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <checkup/failure.h>
#include <checkup/test.h>

#include <libuserspace/memory.h>
#include <libuserspace/printf.h>

#include <muzzle/string.h>
#include <muzzle/stdlib.h>

Test::Test(const char* name) {
    mName = (char*)calloc(1, strlen(name) + 1);
    strcpy(mName, name);
}

const char* Test::name() const {
    return mName;
}

bool Test::setup() {
    return true;
}

void Test::teardown() {}

void Test::test() {
    if (false == setup()) {
        FAIL("setup failed");
    }

    run();

    teardown();

    printf("TEST[%s] passed!\n", name());
}

Test::~Test() {
    free(mName);
}
