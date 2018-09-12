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

#include "../include/document.h"

void Document::move(size_t newLine) {
    mIndex = newLine;
    if (mIndex > mLines.size()) mIndex = mLines.size();
}

void Document::insert(const eastl::string& line) {
    if (mIndex == mLines.size()) {
        mLines.push_back(line);
    } else {
        mLines.insert(mLines.begin() + mIndex, line);
    }

    ++mIndex;
}

eastl::string Document::toString() {
    eastl::string out;
    bool first = true;

    for (const auto& line : mLines) {
        if (first) first = false;
        else out += "\n";
        out += line;
    }

    return out;
}

eastl::vector<eastl::string> Document::toLines() {
    return mLines;
}
