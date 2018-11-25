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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>

static const char* THE_COW =
R"X(
       \   ^__^
        \  (oo)\_______
           (__)\       )\/\
               ||----w |
               ||     ||)X";

char* readFile(FILE* f) {
    std::string str;
    while(true) {
        int c = fgetc(f);
        if (c == EOF) break;
        str.append_sprintf("%c", c);
    }
    fclose(f);
    return strdup(str.c_str());
}

char* readFile(const char* path) {
    FILE* fd = fopen(path, "r");
    if (fd == nullptr) return nullptr;
    return readFile(fd);
}

size_t maxLength(const char* data) {
    size_t max = 0;
    size_t len = 0;
    for(; *data; ++data) {
        if (*data == '\n') {
            if (len > max) {
                max = len;
            }
            len = 0;
        } else {
            ++len;
        }
    }
    
    if (max < 10) max = 10;
    if (max > 78) max = 78;
    return max;
}

void printRepeated(char c, size_t len, bool nl) {
    putchar(' ');
    for(; len != 0; --len) {
        putchar(c);
    }
    if (nl) putchar('\n');
}

enum class eCurrentMode {
    START_LINE,
    CONSUME_BUFFER,
    END_LINE
};

static bool isEndOfText(const char* buffer) {
    const char* eol = strchr(buffer, '\n');
    if (eol == nullptr) return true;
    if (*(eol + 1) == 0) return true;

    return false;
}

static size_t nextWordLen(const char* buffer) {
    const char* nextSeparator = strpbrk(buffer, "\n \t");
    if (nextSeparator == nullptr) return 0;
    return nextSeparator - buffer;
}

void printBuffer(const char* buffer, size_t max) {
    const size_t total = strlen(buffer);
    size_t residual = total;
    size_t nline = 0;
    size_t written = 0;
    bool is_first_line = false;
    bool is_last_line = false;
    eCurrentMode mode = eCurrentMode::START_LINE;
    while(true) {
        if (mode == eCurrentMode::START_LINE) {
            is_first_line = nline == 0;
            is_last_line = (residual <= max) && isEndOfText(buffer);
            if (is_first_line) {
                putchar('/');
            } else if (is_last_line) {
                putchar('\\');
            } else {
                putchar('|');
            }
            putchar(' ');
            written = 0;
            mode = eCurrentMode::CONSUME_BUFFER;
            continue;
        }
        
        if (mode == eCurrentMode::CONSUME_BUFFER) {
            bool is_space = false;
            if (written == max || *buffer == 0) {
                mode = eCurrentMode::END_LINE;
            } else if (*buffer == '\n') {
                ++buffer;
                mode = eCurrentMode::END_LINE;
            } else if (*buffer == '\t') {
                ++buffer;
                printf("    ");
                written += 4;
                is_space = true;
            } else if (*buffer == ' ') {
                ++buffer;
                putchar(' ');
                ++written;
                is_space = true;
            } else {
                putchar(*buffer);
                ++buffer;
                ++written; --residual;
            }
            if (is_space) {
                auto wl = nextWordLen(buffer);
                if (wl == 0) continue;
                if (wl > (max-written)) mode = eCurrentMode::END_LINE;
            }
            continue;
        }
        
        if (mode == eCurrentMode::END_LINE) {
            for(; written <= max; ++written) putchar(' ');
            if (is_first_line) {
                putchar('\\');
            } else if(is_last_line) {
                putchar('/');
            } else {
                putchar('|');
            }
            putchar('\n');
            ++nline;
            if (*buffer == 0) break;
            mode = eCurrentMode::START_LINE;
            continue;
        }
    }
}

int main(int argc, const char * argv[]) {
    const char* content = (argc == 1 ?
                           readFile(stdin) :
                           readFile(argv[1]));

    size_t max = maxLength(content);
    printRepeated('_', max+2, true);
    printBuffer(content, max);
    printRepeated('-', max+2, false);
    printf("%s\n", THE_COW);
    return 0;
}
