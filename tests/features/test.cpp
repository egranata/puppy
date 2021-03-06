/*
 * Copyright 2019 Google LLC
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

#include <libcheckup/test.h>
#include <libcheckup/assert.h>
#include <syscalls.h>
#include <kernel/syscalls/types.h>
#include <sys/osfeatures.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            feature_id_t features[] = {gFeatureIdExecInheritsSystem, gFeatureIdPuppy};
            CHECK_EQ(0, checkfeatures_syscall(features));
            features[0] = gFeatureIdInvalid;
            CHECK_NOT_EQ(0, checkfeatures_syscall(features));
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
