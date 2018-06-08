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

#ifndef LIBC_STR
#define LIBC_STR

#include <sys/stdint.h>

class string {
    public:
        string(char c = 0, size_t n = 0);
        explicit string(const char* buffer);
        string(const string&);
        string(string&&);
        ~string();

        const char* c_str() const;
        size_t size() const;

        typedef char* iterator;
        typedef const char* const_iterator;

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

        char& operator[](size_t idx);

        string& operator=(const char*);
        string& operator=(const string&);
        string& operator=(string&&);

        bool operator==(const string&);

        string operator+(const string&);

        void reset(const char* buf);
    private:
        char* mBuffer;
};

#endif