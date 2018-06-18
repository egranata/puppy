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
#include <libuserspace/exit.h>
#include <libuserspace/file.h>

static void usage(bool exit) {
    printf("cp <src> <dst>\n");
    if (exit) ::exit(1);
}

void copy(int from, int to) {
    uint8_t buffer[1024] = {0};

    while(true) {
        size_t n = ::read(from, 1024, buffer);
        ::write(to, n, buffer);
        if (n < 1024) break;
    }
}

int main(int argc, const char** argv) {
    if (argc != 2) {
        usage(true);
    }

    auto srcFd = open(argv[0], FILE_OPEN_READ);
    if (srcFd == gInvalidFd) usage(true);
    auto dstFd = open(argv[1], FILE_OPEN_WRITE | FILE_OPEN_NEW);
    if (dstFd == gInvalidFd) usage(true);

    copy(srcFd, dstFd);

    close(srcFd);
    close(dstFd);

    return 0;
}