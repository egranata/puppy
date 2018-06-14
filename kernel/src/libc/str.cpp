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

#include <kernel/libc/str.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>

void string::reset(const char* buffer) {
    free(mBuffer);
    if (buffer) {
        auto l = strlen(buffer);
        mBuffer = allocate<char>(l + 1);
        strcpy(mBuffer, buffer);
    } else {
        mBuffer = nullptr;
    }
}

string::string(char c, size_t n) {
    if (n == 0) {
        mBuffer = nullptr;
    } else {
        mBuffer = allocate<char>(n+1);
        mBuffer[n] = 0;
        for (size_t i = 0; i < n; ++i) {
            mBuffer[i] = c;
        }
    }
}

string::string(const char* buffer) : mBuffer(nullptr) {
    reset(buffer);
}

string::string(const string& str) : string((const char*)str.mBuffer) {}
string::string(string&& str) {
    mBuffer = str.mBuffer;
    str.mBuffer = nullptr;
}

string::~string() {
    reset(nullptr);
}

const char* string::c_str() const {
    return mBuffer;
}

size_t string::size() const {
    return mBuffer ? strlen(mBuffer) : 0;
}

string::iterator string::begin() {
    if (mBuffer) {
        return (iterator)&mBuffer[0];
    } else {
        return nullptr;
    }
}
string::iterator string::end() {
    if (mBuffer) {
        return (iterator)&mBuffer[size() + 1];
    } else {
        return nullptr;
    }
}

char& string::operator[](size_t idx) {
    return *(mBuffer + idx);
}

string& string::operator=(const char* buffer) {
    reset(buffer);
    return *this;
}

string& string::operator=(const string& str) {
    reset(str.mBuffer);
    return *this;
}

string& string::operator=(string&& str) {
    mBuffer = str.mBuffer;
    str.mBuffer = nullptr;
    return *this;
}

bool string::operator==(const string& str) {
    if (mBuffer) {
        if (str.mBuffer) {
            return 0 == strcmp(mBuffer, str.mBuffer);
        } else return false;
    } else return (nullptr == str.mBuffer);
}

string string::operator+(const string& str) {
    if (mBuffer) {
        if (str.mBuffer) {
            string s;
            auto ts = size();
            auto ss = str.size();
            auto buf = allocate<char>(ts + ss + 1);
            strncpy(buf, mBuffer, ts);
            strncat(buf, str.mBuffer, ss);
            s.mBuffer = buf;
            return s;
        } else return string(*this);
    } else {
        if (str.mBuffer) {
            return string(str);
        } else return string(nullptr);
    }
}
