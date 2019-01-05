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

#ifndef CHECKUP_ASSERT
#define CHECKUP_ASSERT

#define CHECK_EQ(x,y) __do_check((x == y), name(), __FILE__, __LINE__, #x " == " #y)

#define CHECK_NOT_EQ(x,y) __do_check((x != y), name(), __FILE__, __LINE__, #x " != " #y)

#define CHECK_NULL(x) __do_check((x == nullptr), name(), __FILE__, __LINE__, #x " == nullptr")

#define CHECK_NOT_NULL(x) __do_check((x != nullptr), name(), __FILE__, __LINE__, #x " != nullptr")

#define CHECK_TRUE(x) __do_check((x), name(), __FILE__, __LINE__, #x)

#define CHECK_FALSE(x) __do_check((!x), name(), __FILE__, __LINE__, "!(" #x ")")

void __do_check(bool ok, const char* test, const char* file, int line, const char* condition);

#endif
