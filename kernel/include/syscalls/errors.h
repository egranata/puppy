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

#ifndef DEFINE_ERROR
#error "define DEFINE_ERROR before including this file"
#endif

DEFINE_ERROR(NO_SUCH_SYSCALL, 1);
DEFINE_ERROR(NO_SUCH_DEVICE, 2);
DEFINE_ERROR(MSG_QUEUE_EMPTY, 3);
DEFINE_ERROR(NO_SUCH_PROCESS, 4);
DEFINE_ERROR(OUT_OF_MEMORY, 5);
DEFINE_ERROR(DISK_IO_ERROR, 6);
DEFINE_ERROR(NO_SUCH_FILE, 7);
DEFINE_ERROR(UNIMPLEMENTED, 8);
DEFINE_ERROR(OUT_OF_HANDLES, 9);
DEFINE_ERROR(NO_SUCH_OBJECT, 10);
DEFINE_ERROR(NOT_ALLOWED, 11);
DEFINE_ERROR(ALREADY_LOCKED, 12);
DEFINE_ERROR(NOT_A_FILE, 13);
DEFINE_ERROR(TIMEOUT, 14);
