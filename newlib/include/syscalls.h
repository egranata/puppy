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

// autogenerated by syscalls/genimply.do - DO NOT EDIT manually

#ifndef NEWLIB_SYSCALLS
#define NEWLIB_SYSCALLS

#include <kernel/syscalls/types.h>

syscall_response_t yield_syscall();
constexpr uint8_t yield_syscall_id = 0x1;
syscall_response_t sleep_syscall(uint32_t arg1);
constexpr uint8_t sleep_syscall_id = 0x2;
syscall_response_t exit_syscall(uint8_t arg1);
constexpr uint8_t exit_syscall_id = 0x3;
syscall_response_t halt_syscall();
constexpr uint8_t halt_syscall_id = 0x4;
syscall_response_t reboot_syscall();
constexpr uint8_t reboot_syscall_id = 0x5;
syscall_response_t sysinfo_syscall(sysinfo_t* arg1,uint32_t arg2);
constexpr uint8_t sysinfo_syscall_id = 0x6;
syscall_response_t getcurdir_syscall(char* arg1,size_t* arg2);
constexpr uint8_t getcurdir_syscall_id = 0x7;
syscall_response_t setcurdir_syscall(const char* arg1);
constexpr uint8_t setcurdir_syscall_id = 0x8;
syscall_response_t getpid_syscall();
constexpr uint8_t getpid_syscall_id = 0x9;
syscall_response_t fopen_syscall(const char* arg1,uint32_t arg2);
constexpr uint8_t fopen_syscall_id = 0xa;
syscall_response_t fclose_syscall(uint32_t arg1);
constexpr uint8_t fclose_syscall_id = 0xb;
syscall_response_t fdup_syscall(uint32_t arg1);
constexpr uint8_t fdup_syscall_id = 0xc;
syscall_response_t fread_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3);
constexpr uint8_t fread_syscall_id = 0xd;
syscall_response_t exec_syscall(const char* arg1,char** arg2,char** arg3,uint32_t arg4,exec_fileop_t* arg5);
constexpr uint8_t exec_syscall_id = 0xe;
syscall_response_t kill_syscall(uint32_t arg1);
constexpr uint8_t kill_syscall_id = 0xf;
syscall_response_t fstat_syscall(uint32_t arg1,uint32_t arg2);
constexpr uint8_t fstat_syscall_id = 0x10;
syscall_response_t fseek_syscall(uint32_t arg1,uint32_t arg2);
constexpr uint8_t fseek_syscall_id = 0x11;
syscall_response_t ftell_syscall(uint16_t arg1,size_t* arg2);
constexpr uint8_t ftell_syscall_id = 0x12;
syscall_response_t fopendir_syscall(uint32_t arg1);
constexpr uint8_t fopendir_syscall_id = 0x13;
syscall_response_t freaddir_syscall(uint16_t arg1,file_info_t* arg2);
constexpr uint8_t freaddir_syscall_id = 0x14;
syscall_response_t getppid_syscall();
constexpr uint8_t getppid_syscall_id = 0x15;
syscall_response_t collect_syscall(kpid_t arg1,process_exit_status_t* arg2);
constexpr uint8_t collect_syscall_id = 0x16;
syscall_response_t fioctl_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3);
constexpr uint8_t fioctl_syscall_id = 0x17;
syscall_response_t fwrite_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3);
constexpr uint8_t fwrite_syscall_id = 0x18;
syscall_response_t prioritize_syscall(kpid_t arg1,prioritize_target arg2,const exec_priority_t* arg3,exec_priority_t* arg4);
constexpr uint8_t prioritize_syscall_id = 0x19;
syscall_response_t mapregion_syscall(uint32_t arg1,uint32_t arg2);
constexpr uint8_t mapregion_syscall_id = 0x1a;
syscall_response_t unmapregion_syscall(uint32_t arg1);
constexpr uint8_t unmapregion_syscall_id = 0x1b;
syscall_response_t setregionperms_syscall(uint32_t arg1,uint32_t arg2);
constexpr uint8_t setregionperms_syscall_id = 0x1c;
syscall_response_t trymount_syscall(uint32_t arg1,const char* arg2);
constexpr uint8_t trymount_syscall_id = 0x1d;
syscall_response_t collectany_syscall(bool arg1,kpid_t* arg2,process_exit_status_t* arg3);
constexpr uint8_t collectany_syscall_id = 0x1e;
syscall_response_t clone_syscall(uintptr_t arg1,exec_fileop_t* arg2);
constexpr uint8_t clone_syscall_id = 0x1f;
syscall_response_t fdel_syscall(const char* arg1);
constexpr uint8_t fdel_syscall_id = 0x20;
syscall_response_t mkdir_syscall(const char* arg1);
constexpr uint8_t mkdir_syscall_id = 0x21;
syscall_response_t proctable_syscall(process_info_t* arg1,size_t arg2);
constexpr uint8_t proctable_syscall_id = 0x22;
syscall_response_t vmcheckreadable_syscall(uintptr_t arg1,size_t arg2);
constexpr uint8_t vmcheckreadable_syscall_id = 0x23;
syscall_response_t vmcheckwritable_syscall(uintptr_t arg1,size_t arg2);
constexpr uint8_t vmcheckwritable_syscall_id = 0x24;
syscall_response_t pipe_syscall(size_t* arg1,size_t* arg2);
constexpr uint8_t pipe_syscall_id = 0x25;

#endif