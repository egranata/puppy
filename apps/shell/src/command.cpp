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

#include "command.h"

#include <stdio.h>
#include <unistd.h>

#include <libshell/expand.h>

#include <sys/wait.h>
#include <sys/process.h>

Redirect::Redirect(const char* target) : mTarget(target ? target : "") {}
const char* Redirect::target() const { return mTarget.c_str(); }

Command::Command() = default;
Command::Command(const Command& rhs) {
    auto wb = rhs.mWords.begin();
    auto we = rhs.mWords.end();
    for (; wb != we; ++wb) {
        addWord(wb->c_str());
    }
    if (rhs.mOutRedirect && rhs.mOutRedirect->target()) redirectStdout(rhs.mOutRedirect->target());
    if (rhs.mInRedirect && rhs.mInRedirect->target()) redirectStdin(rhs.mInRedirect->target());
    if (rhs.mPipeTarget) mPipeTarget = rhs.mPipeTarget;
    mBackground = rhs.mBackground;
}

void Command::setBackground(bool b) {
    mBackground = b;
}
void Command::addWord(const char* word) {
    if (word && word[0]) mWords.push_back(word);
}
void Command::redirectStdout(const char* target) {
    mOutRedirect = Redirect(target);
}
void Command::redirectStdin(const char* target) {
    mInRedirect = Redirect(target);
}
void Command::setPipe(const Command& cmd) {
    mPipeTarget.reset(new Command(cmd));
}
void Command::clear() {
    mWords.clear();
    mOutRedirect.reset();
    mInRedirect.reset();
    mPipeTarget.reset();
}

Command::operator bool() const {
    return false == mWords.empty();
}

void Command::printf() const {
    for (const auto& word : mWords) {
        ::printf("%s ", word.c_str());
    }
    if (mOutRedirect) {
        ::printf("> %s ", mOutRedirect->target());
    }
    if (mInRedirect) {
        ::printf("< %s ", mInRedirect->target());
    }
    if (mBackground) {
        ::printf("& ");
    }
    if (mPipeTarget) {
        ::printf("| ");
        mPipeTarget->printf();
    }
}
