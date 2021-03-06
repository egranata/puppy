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

#include <libcheckup/failure.h>
#include <libcheckup/klog.h>

#include <newlib/stdlib.h>
#include <newlib/stdio.h>

void __failed(const char* test, const char* file, int line, const char* condition) {
    fprintf(getLogFile(), "[TEST[%s] FAIL condition failed: %s at %s:%d", test, condition, file, line);
    printf("TEST[%s] \x1b[31mFAIL\x1b[0m condition failed: %s at %s:%d\n", test, condition, file, line);
    exit(1);
}
