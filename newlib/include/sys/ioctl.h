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

#ifndef NEWLIB_IOCTL
#define NEWLIB_IOCTL

#include <newlib/sys/errno.h>
#include <newlib/stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIOCGWINSZ 0x40087468

struct winsize {
  unsigned short ws_row;	/* rows, in characters */
  unsigned short ws_col;	/* columns, in characters */
  unsigned short ws_xpixel;	/* horizontal size, pixels */
  unsigned short ws_ypixel;	/* vertical size, pixels */
};

int ioctl(int fd, int a, int b);

#ifdef __cplusplus
}
#endif

#endif
