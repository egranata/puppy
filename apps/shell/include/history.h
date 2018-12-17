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

#ifndef SHELL_HISTORY
#define SHELL_HISTORY

#include <string>

class History {
    public:
        static constexpr const char* gDefaultHistoryFile = "/home/shell.history";

        History(const char* path = nullptr);
        void save();
        ~History();

        void add(const std::string& s);

        static History& defaultHistory();
    private:
        std::string mFilePath;
};

#endif
