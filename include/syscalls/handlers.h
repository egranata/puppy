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

#ifndef SYSCALLS_HANDLERS
#define SYSCALLS_HANDLERS

#include <sys/stdint.h>
#include <syscalls/manager.h>

#define ERR(name) SyscallManager::SYSCALL_ERR_ ## name
#define OK SyscallManager::SYSCALL_SUCCESS

#define HANDLER0(name) syscall_response_t name ## _syscall_handler()
#define HANDLER1(name, arg1) syscall_response_t name ## _syscall_handler(uint32_t arg1)
#define HANDLER2(name, arg1, arg2) syscall_response_t name ## _syscall_handler(uint32_t arg1, uint32_t arg2)
#define HANDLER3(name, arg1, arg2, arg3) syscall_response_t name ## _syscall_handler(uint32_t arg1, uint32_t arg2, uint32_t arg3)
#define HANDLER4(name, arg1, arg2, arg3, arg4) syscall_response_t name ## _syscall_handler(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)

#endif
