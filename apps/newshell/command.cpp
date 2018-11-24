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
    if (rhs.mRedirect && rhs.mRedirect->target()) setRedirect(rhs.mRedirect->target());
    if (rhs.mPipeTarget) mPipeTarget = rhs.mPipeTarget;
    mBackground = rhs.mBackground;
}

void Command::setBackground(bool b) {
    mBackground = b;
}
void Command::addWord(const char* word) {
    if (word && word[0]) mWords.push_back(word);
}
void Command::setRedirect(const char* target) {
    mRedirect = Redirect(target);
}
void Command::setPipe(const Command& cmd) {
    mPipeTarget.reset(new Command(cmd));
}
void Command::clear() {
    mWords.clear();
    mRedirect.reset();
    mPipeTarget.reset();
}

void Command::printf() const {
    for (const auto& word : mWords) {
        ::printf("%s ", word.c_str());
    }
    if (mRedirect) {
        ::printf("> %s ", mRedirect->target());
    }
    if (mBackground) {
        ::printf("& ");
    }
    if (mPipeTarget) {
        ::printf("| ");
        mPipeTarget->printf();
    }
}

void Command::exec() {
    if (mPipeTarget) {
        fprintf(stderr, "pipe: unimplemented\n");
    }

    char** argv = (char**)calloc(mWords.size() + 1, sizeof(char*));
    for (size_t i = 0; i < mWords.size(); ++i) argv[i] = (char*)mWords[i].c_str();

    exec_fileop_t fops[] = {
        exec_fileop_t{
            .op = exec_fileop_t::operation::END_OF_LIST,
            .param1 = 0,
            .param2 = 0,
            .param3 = nullptr
        },
        exec_fileop_t{
            .op = exec_fileop_t::operation::END_OF_LIST,
            .param1 = 0,
            .param2 = 0,
            .param3 = nullptr,
        },
        exec_fileop_t{
            .op = exec_fileop_t::operation::END_OF_LIST,
            .param1 = 0,
            .param2 = 0,
            .param3 = nullptr
        },
    };

    FILE *rdest = nullptr;
    if (mRedirect) {
        rdest = fopen(mRedirect->target(), "w");
        if (rdest) {
            fops[0].op = exec_fileop_t::operation::CLOSE_CHILD_FD;
            fops[0].param1 = STDOUT_FILENO;
            fops[1].op = exec_fileop_t::operation::DUP_PARENT_FD;
            fops[1].param1 = fileno(rdest);
        } else {
            fprintf(stderr, "redirect: failed\n");
        }
    }

    auto chld = spawn(mWords[0].c_str(),
        argv,
        PROCESS_INHERITS_CWD | (mBackground ? SPAWN_BACKGROUND : SPAWN_FOREGROUND) | PROCESS_INHERITS_ENVIRONMENT,
        fops);
    if (rdest) fclose(rdest);
    if (mBackground) {
        ::printf("[child %u] spawned\n", chld);
    } else {
        int exitcode = 0;
        waitpid(chld, &exitcode, 0);
    }
}
