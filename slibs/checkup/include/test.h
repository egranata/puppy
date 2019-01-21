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

#ifndef CHECKUP_TEST
#define CHECKUP_TEST

#include <string>
#include <unordered_map>
#include <stdio.h>

class Test {
    public:
        const char* name() const;

        void test();
    protected:
        Test(const char* name);

        virtual bool setup();

        virtual void run() = 0;

        virtual void teardown();
        ~Test();

        // return a semaphore fd in a path safe for this test case to use
        // the file descriptor will not be released until the test has finished
        int getSemaphore(const char* name);

        // return a temporary file managed by this test case and that will be deleted
        // at the end of the test case, regardless of test success (but not in case of
        // a crash)
        const char* getTempFile(const char* key);

        size_t writeString(FILE*, const char*);
        const char* checkReadString(FILE*, const char*);

    private:
        void cleanupResources();

        std::string mName;
        std::unordered_map<std::string, FILE*> mSemaphores;
        std::unordered_map<std::string, std::string> mTemporaryFiles;
};

#endif
