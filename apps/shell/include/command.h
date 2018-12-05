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

#ifndef SHELL_COMMAND
#define SHELL_COMMAND

#include <string>
#include <memory>
#include <vector>
#include <optional>

class Redirect {
    public:
        Redirect(const char* target);
        const char* target() const;
    private:
        std::string mTarget;
};

class Command {
    public:
        Command();
        Command(const Command&);
        void setBackground(bool = true);
        void addWord(const char* word);
        void redirectStdout(const char* target);
        void redirectStdin(const char* target);
        void setPipe(const Command& cmd);
        void setSuccessChain(const Command& cmd);
        void clear();

        bool background() const { return mBackground; }
        const auto& pipe() const { return mPipeTarget; }
        const auto& success() const { return mRunIfSuccess; }
        const auto begin() const { return mWords.begin(); }
        const auto end() const { return mWords.end(); }
        const auto& stdoutR() const { return mOutRedirect; }
        const auto& stdinR() const { return mInRedirect; }

        explicit operator bool() const;

        void exec();
        void printf() const;
    private:
        std::vector<std::string> mWords;
        std::optional<Redirect> mOutRedirect;
        std::optional<Redirect> mInRedirect;
        std::shared_ptr<Command> mPipeTarget;
        std::shared_ptr<Command> mRunIfSuccess;
        bool mBackground = false;
};


#endif
