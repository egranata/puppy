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

#ifndef PUPPED_DOCUMENT
#define PUPPED_DOCUMENT

#include <EASTL/string.h>
#include <EASTL/vector.h>

class Document {
    public:
        Document() : mIndex(0), mLines() {}

        size_t numLines() { return mLines.size(); }
        bool empty() { return mLines.empty(); }
        size_t currentLine() { return mIndex; }

        void move(size_t newLine);
        void insert(const eastl::string& line);
        eastl::string toString();
    private:
        size_t mIndex;
        eastl::vector<eastl::string> mLines;
};

#endif
