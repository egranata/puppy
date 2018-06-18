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

#include <libuserspace/printf.h>
#include <libuserspace/file.h>
#include <libuserspace/exit.h>
#include <libuserspace/memory.h>

const unsigned char* read(uint32_t fd, uint32_t& size) {
    size = 0;
    if (!fsize(fd, size)) {
        return nullptr;
    }
    auto buffer = (unsigned char*)malloc(size);
    if (read(fd, size, buffer)) return buffer;
    return nullptr;
}

void dump(const unsigned char* buffer, uint32_t size) {
    for (auto i = 0u; true; i+= 16) {
        if (i >= size) break;
        printf("%04x ", i);
        for (auto j = 0; j < 16; ++j) {
            printf("%02x ", buffer[i + j]);
        }
        printf("    ");
        for (auto j = 0; j < 16; ++j) {
            if (i + j >= size) break;
            auto c = buffer[i + j];
            switch(c) {
                case '\n':
                    printf("\\n");
                    break;
                case 0:
                    printf("\\0");
                    break;
                default:
                    printf("%c", buffer[i + j]);
                    break;
            }
        }
        printf("\n");
    }
}

void cat(const char* path) {
    printf("Printout of %s\n", path);
    printf("==============================================================================\n");
    auto fd = open(path, gModeRead);
    if (fd == gInvalidFd) {
        printf("could not open %s - exiting\n", path);
        exit(1);
    }

    uint32_t filesize = 0;
    auto buffer = read(fd, filesize);
    if (buffer == nullptr) {
        printf("could not read %s - exiting\n", path);
        exit(1);
    }

    dump(buffer, filesize);
    printf("==============================================================================\n");    
}

int main(int argc, const char** argv) {
    for (auto i = 0; i < argc; ++i) {
        cat(argv[i]);
    }

    return 0;
}
