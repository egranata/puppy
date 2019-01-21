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

#include <libcheckup/assert.h>
#include <libcheckup/failure.h>
#include <libcheckup/test.h>
#include <libcheckup/success.h>

#include <unistd.h>

Test::Test(const char* name) : mName(name ? name : "TEST") {}

const char* Test::name() const {
    return mName.c_str();
}

bool Test::setup() {
    return true;
}

void Test::teardown() {}

void Test::test() {
    if (false == setup()) {
        cleanupResources();
        FAIL("setup failed");
    } else {
        run();
        teardown();
        cleanupResources();
        __success(name());
    }
}

int Test::getSemaphore(const char* sn) {
    std::string sema_path;
    sema_path.append_sprintf("/semaphores/checkup/%s/%s", name(), sn);
    auto i = mSemaphores.find(sema_path), e = mSemaphores.end();
    if (i == e) {
        FILE *f = fopen(sema_path.c_str(), "r");
        if (f) {
            int fd = fileno(f);
            mSemaphores.emplace(sema_path, f);
            return fd;
        } else return -1;
    }
    return fileno(i->second);
}

const char* Test::getTempFile(const char* key) {
    auto i = mTemporaryFiles.find(key), e = mTemporaryFiles.end();
    if (i == e) {
        std::string file_path;
        file_path.append_sprintf("/tmp/test.%s.%s", name(), key);
        mTemporaryFiles.emplace(key, file_path);
        return getTempFile(key);
    } else return i->second.c_str();
}

void Test::cleanupResources() {
    for(auto i = mSemaphores.begin(); i != mSemaphores.end(); ++i)
        fclose(i->second);
    mSemaphores.clear();
    for (auto i = mTemporaryFiles.begin(); i != mTemporaryFiles.end(); ++i)
        unlink(i->second.c_str());
    mTemporaryFiles.clear();
}

Test::~Test() {
    cleanupResources();
}

size_t Test::writeString(FILE* fd, const char* s) {
    CHECK_NOT_EQ(fd, nullptr);
    auto ls = strlen(s);
    return fwrite(s, 1, ls, fd);
}

const char* Test::checkReadString(FILE* fd, const char* expected) {
    static constexpr size_t max_size = 4096;

    CHECK_NOT_EQ(fd, nullptr);
    uint8_t *buffer = (uint8_t *)malloc(max_size);
    bzero(buffer, max_size);
    CHECK_NOT_EQ(fread(buffer, 1, max_size - 1, fd), 0);

    CHECK_EQ(strcmp((const char*)buffer, expected), 0);

    return (const char*)buffer;
}
