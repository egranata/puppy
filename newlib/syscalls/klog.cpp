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

#include <newlib/stdlib.h>
#include <newlib/string.h>
#include <newlib/sys/unistd.h>
#include <newlib/impl/klog.h>
#include <newlib/impl/cenv.h>
#include <stdarg.h>
#include <newlib/syscalls.h>
#include <newlib/stdio.h>

void newlib::puppy::impl::klog(const char* fmt, ...) {
    static FILE* klog = nullptr;

    if (klog == nullptr) {
        klog = fopen("/devices/klog", "w");
    }
    if (klog) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(klog, fmt, ap);
        va_end(ap);
    }
}
