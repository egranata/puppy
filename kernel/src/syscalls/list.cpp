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

#include <kernel/syscalls/manager.h>
#include <kernel/syscalls/types.h>
#include <kernel/libc/string.h>

extern syscall_response_t yield_syscall_handler();
extern syscall_response_t yield_syscall_helper(SyscallManager::Request&);
extern syscall_response_t sleep_syscall_handler(uint32_t arg1);
extern syscall_response_t sleep_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t msgsend_syscall_handler(uint32_t arg1,uint32_t arg2,uint32_t arg3);
extern syscall_response_t msgsend_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t msgrecv_syscall_handler(message_t* arg1,bool arg2);
extern syscall_response_t msgrecv_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t exit_syscall_handler(uint8_t arg1);
extern syscall_response_t exit_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t reboot_syscall_handler();
extern syscall_response_t reboot_syscall_helper(SyscallManager::Request&);
extern syscall_response_t sysinfo_syscall_handler(sysinfo_t* arg1,uint32_t arg2);
extern syscall_response_t sysinfo_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t getcurdir_syscall_handler(char* arg1,size_t* arg2);
extern syscall_response_t getcurdir_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t setcurdir_syscall_handler(const char* arg1);
extern syscall_response_t setcurdir_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t semwait_syscall_handler(uint32_t arg1);
extern syscall_response_t semwait_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t semsignal_syscall_handler(uint32_t arg1);
extern syscall_response_t semsignal_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t getpid_syscall_handler();
extern syscall_response_t getpid_syscall_helper(SyscallManager::Request&);
extern syscall_response_t fopen_syscall_handler(const char* arg1,uint32_t arg2);
extern syscall_response_t fopen_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t fclose_syscall_handler(uint32_t arg1);
extern syscall_response_t fclose_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t fdup_syscall_handler(uint32_t arg1);
extern syscall_response_t fdup_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t fread_syscall_handler(uint32_t arg1,uint32_t arg2,uint32_t arg3);
extern syscall_response_t fread_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t exec_syscall_handler(const char* arg1,const char* arg2,char** arg3,uint32_t arg4);
extern syscall_response_t exec_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t kill_syscall_handler(uint32_t arg1);
extern syscall_response_t kill_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t fstat_syscall_handler(uint32_t arg1,uint32_t arg2);
extern syscall_response_t fstat_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t fseek_syscall_handler(uint32_t arg1,uint32_t arg2);
extern syscall_response_t fseek_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t fopendir_syscall_handler(uint32_t arg1);
extern syscall_response_t fopendir_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t freaddir_syscall_handler(uint16_t arg1,file_info_t* arg2);
extern syscall_response_t freaddir_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t getppid_syscall_handler();
extern syscall_response_t getppid_syscall_helper(SyscallManager::Request&);
extern syscall_response_t collect_syscall_handler(uint16_t arg1,process_exit_status_t* arg2);
extern syscall_response_t collect_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t semget_syscall_handler(uint32_t arg1);
extern syscall_response_t semget_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t fioctl_syscall_handler(uint32_t arg1,uint32_t arg2,uint32_t arg3);
extern syscall_response_t fioctl_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t fwrite_syscall_handler(uint32_t arg1,uint32_t arg2,uint32_t arg3);
extern syscall_response_t fwrite_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t prioritize_syscall_handler(uint32_t arg1,uint32_t arg2);
extern syscall_response_t prioritize_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t mutexget_syscall_handler(uint32_t arg1);
extern syscall_response_t mutexget_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t mutexlock_syscall_handler(uint32_t arg1);
extern syscall_response_t mutexlock_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t mutexunlock_syscall_handler(uint32_t arg1);
extern syscall_response_t mutexunlock_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t mapregion_syscall_handler(uint32_t arg1,uint32_t arg2);
extern syscall_response_t mapregion_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t unmapregion_syscall_handler(uint32_t arg1);
extern syscall_response_t unmapregion_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t setregionperms_syscall_handler(uint32_t arg1,uint32_t arg2);
extern syscall_response_t setregionperms_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t trymount_syscall_handler(uint32_t arg1,const char* arg2);
extern syscall_response_t trymount_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t collectany_syscall_handler(uint16_t* arg1,process_exit_status_t* arg2);
extern syscall_response_t collectany_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t clone_syscall_handler(uintptr_t arg1);
extern syscall_response_t clone_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t fdel_syscall_handler(const char* arg1);
extern syscall_response_t fdel_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t mkdir_syscall_handler(const char* arg1);
extern syscall_response_t mkdir_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t klog_syscall_handler(const char* arg1);
extern syscall_response_t klog_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t klogread_syscall_handler(char* arg1,size_t arg2,klog_stats_t* arg3);
extern syscall_response_t klogread_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t proctable_syscall_handler(process_info_t* arg1,size_t arg2);
extern syscall_response_t proctable_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t mutextrylock_syscall_handler(uint32_t arg1);
extern syscall_response_t mutextrylock_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t vmcheckreadable_syscall_handler(uintptr_t arg1,size_t arg2);
extern syscall_response_t vmcheckreadable_syscall_helper(SyscallManager::Request& req);
extern syscall_response_t vmcheckwritable_syscall_handler(uintptr_t arg1,size_t arg2);
extern syscall_response_t vmcheckwritable_syscall_helper(SyscallManager::Request& req);

void SyscallManager::sethandlers() {
	handle(1, yield_syscall_helper, false); 
	handle(2, sleep_syscall_helper, false); 
	handle(3, msgsend_syscall_helper, false); 
	handle(4, msgrecv_syscall_helper, false); 
	handle(5, exit_syscall_helper, false); 
	handle(6, reboot_syscall_helper, false); 
	handle(7, sysinfo_syscall_helper, false); 
	handle(8, getcurdir_syscall_helper, false); 
	handle(9, setcurdir_syscall_helper, false); 
	handle(10, semwait_syscall_helper, false); 
	handle(11, semsignal_syscall_helper, false); 
	handle(12, getpid_syscall_helper, false); 
	handle(13, fopen_syscall_helper, false); 
	handle(14, fclose_syscall_helper, false); 
	handle(15, fdup_syscall_helper, false); 
	handle(16, fread_syscall_helper, false); 
	handle(17, exec_syscall_helper, false); 
	handle(18, kill_syscall_helper, false); 
	handle(19, fstat_syscall_helper, false); 
	handle(20, fseek_syscall_helper, false); 
	handle(21, fopendir_syscall_helper, false); 
	handle(22, freaddir_syscall_helper, false); 
	handle(23, getppid_syscall_helper, false); 
	handle(24, collect_syscall_helper, false); 
	handle(25, semget_syscall_helper, false); 
	handle(26, fioctl_syscall_helper, false); 
	handle(27, fwrite_syscall_helper, false); 
	handle(28, prioritize_syscall_helper, false); 
	handle(29, mutexget_syscall_helper, false); 
	handle(30, mutexlock_syscall_helper, false); 
	handle(31, mutexunlock_syscall_helper, false); 
	handle(32, mapregion_syscall_helper, false); 
	handle(33, unmapregion_syscall_helper, false); 
	handle(34, setregionperms_syscall_helper, false); 
	handle(35, trymount_syscall_helper, false); 
	handle(36, collectany_syscall_helper, false); 
	handle(37, clone_syscall_helper, false); 
	handle(38, fdel_syscall_helper, false); 
	handle(39, mkdir_syscall_helper, false); 
	handle(40, klog_syscall_helper, false); 
	handle(41, klogread_syscall_helper, false); 
	handle(42, proctable_syscall_helper, false); 
	handle(43, mutextrylock_syscall_helper, false); 
	handle(44, vmcheckreadable_syscall_helper, false); 
	handle(45, vmcheckwritable_syscall_helper, false); 
}

syscall_response_t yield_syscall_helper(SyscallManager::Request&) {
	return yield_syscall_handler();
}


syscall_response_t sleep_syscall_helper(SyscallManager::Request& req) {
	return sleep_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t msgsend_syscall_helper(SyscallManager::Request& req) {
	return msgsend_syscall_handler((uint32_t)req.arg1,(uint32_t)req.arg2,(uint32_t)req.arg3);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t msgrecv_syscall_helper(SyscallManager::Request& req) {
	return msgrecv_syscall_handler((message_t*)req.arg1,(bool)req.arg2);
}
static_assert(sizeof(message_t*) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(bool) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t exit_syscall_helper(SyscallManager::Request& req) {
	return exit_syscall_handler((uint8_t)req.arg1);
}
static_assert(sizeof(uint8_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t reboot_syscall_helper(SyscallManager::Request&) {
	return reboot_syscall_handler();
}


syscall_response_t sysinfo_syscall_helper(SyscallManager::Request& req) {
	return sysinfo_syscall_handler((sysinfo_t*)req.arg1,(uint32_t)req.arg2);
}
static_assert(sizeof(sysinfo_t*) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t getcurdir_syscall_helper(SyscallManager::Request& req) {
	return getcurdir_syscall_handler((char*)req.arg1,(size_t*)req.arg2);
}
static_assert(sizeof(char*) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(size_t*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t setcurdir_syscall_helper(SyscallManager::Request& req) {
	return setcurdir_syscall_handler((const char*)req.arg1);
}
static_assert(sizeof(const char*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t semwait_syscall_helper(SyscallManager::Request& req) {
	return semwait_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t semsignal_syscall_helper(SyscallManager::Request& req) {
	return semsignal_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t getpid_syscall_helper(SyscallManager::Request&) {
	return getpid_syscall_handler();
}


syscall_response_t fopen_syscall_helper(SyscallManager::Request& req) {
	return fopen_syscall_handler((const char*)req.arg1,(uint32_t)req.arg2);
}
static_assert(sizeof(const char*) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t fclose_syscall_helper(SyscallManager::Request& req) {
	return fclose_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t fdup_syscall_helper(SyscallManager::Request& req) {
	return fdup_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t fread_syscall_helper(SyscallManager::Request& req) {
	return fread_syscall_handler((uint32_t)req.arg1,(uint32_t)req.arg2,(uint32_t)req.arg3);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t exec_syscall_helper(SyscallManager::Request& req) {
	return exec_syscall_handler((const char*)req.arg1,(const char*)req.arg2,(char**)req.arg3,(uint32_t)req.arg4);
}
static_assert(sizeof(const char*) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(const char*) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(char**) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t kill_syscall_helper(SyscallManager::Request& req) {
	return kill_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t fstat_syscall_helper(SyscallManager::Request& req) {
	return fstat_syscall_handler((uint32_t)req.arg1,(uint32_t)req.arg2);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t fseek_syscall_helper(SyscallManager::Request& req) {
	return fseek_syscall_handler((uint32_t)req.arg1,(uint32_t)req.arg2);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t fopendir_syscall_helper(SyscallManager::Request& req) {
	return fopendir_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t freaddir_syscall_helper(SyscallManager::Request& req) {
	return freaddir_syscall_handler((uint16_t)req.arg1,(file_info_t*)req.arg2);
}
static_assert(sizeof(uint16_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(file_info_t*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t getppid_syscall_helper(SyscallManager::Request&) {
	return getppid_syscall_handler();
}


syscall_response_t collect_syscall_helper(SyscallManager::Request& req) {
	return collect_syscall_handler((uint16_t)req.arg1,(process_exit_status_t*)req.arg2);
}
static_assert(sizeof(uint16_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(process_exit_status_t*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t semget_syscall_helper(SyscallManager::Request& req) {
	return semget_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t fioctl_syscall_helper(SyscallManager::Request& req) {
	return fioctl_syscall_handler((uint32_t)req.arg1,(uint32_t)req.arg2,(uint32_t)req.arg3);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t fwrite_syscall_helper(SyscallManager::Request& req) {
	return fwrite_syscall_handler((uint32_t)req.arg1,(uint32_t)req.arg2,(uint32_t)req.arg3);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t prioritize_syscall_helper(SyscallManager::Request& req) {
	return prioritize_syscall_handler((uint32_t)req.arg1,(uint32_t)req.arg2);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t mutexget_syscall_helper(SyscallManager::Request& req) {
	return mutexget_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t mutexlock_syscall_helper(SyscallManager::Request& req) {
	return mutexlock_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t mutexunlock_syscall_helper(SyscallManager::Request& req) {
	return mutexunlock_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t mapregion_syscall_helper(SyscallManager::Request& req) {
	return mapregion_syscall_handler((uint32_t)req.arg1,(uint32_t)req.arg2);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t unmapregion_syscall_helper(SyscallManager::Request& req) {
	return unmapregion_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t setregionperms_syscall_helper(SyscallManager::Request& req) {
	return setregionperms_syscall_handler((uint32_t)req.arg1,(uint32_t)req.arg2);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t trymount_syscall_helper(SyscallManager::Request& req) {
	return trymount_syscall_handler((uint32_t)req.arg1,(const char*)req.arg2);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(const char*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t collectany_syscall_helper(SyscallManager::Request& req) {
	return collectany_syscall_handler((uint16_t*)req.arg1,(process_exit_status_t*)req.arg2);
}
static_assert(sizeof(uint16_t*) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(process_exit_status_t*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t clone_syscall_helper(SyscallManager::Request& req) {
	return clone_syscall_handler((uintptr_t)req.arg1);
}
static_assert(sizeof(uintptr_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t fdel_syscall_helper(SyscallManager::Request& req) {
	return fdel_syscall_handler((const char*)req.arg1);
}
static_assert(sizeof(const char*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t mkdir_syscall_helper(SyscallManager::Request& req) {
	return mkdir_syscall_handler((const char*)req.arg1);
}
static_assert(sizeof(const char*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t klog_syscall_helper(SyscallManager::Request& req) {
	return klog_syscall_handler((const char*)req.arg1);
}
static_assert(sizeof(const char*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t klogread_syscall_helper(SyscallManager::Request& req) {
	return klogread_syscall_handler((char*)req.arg1,(size_t)req.arg2,(klog_stats_t*)req.arg3);
}
static_assert(sizeof(char*) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(size_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(klog_stats_t*) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t proctable_syscall_helper(SyscallManager::Request& req) {
	return proctable_syscall_handler((process_info_t*)req.arg1,(size_t)req.arg2);
}
static_assert(sizeof(process_info_t*) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(size_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t mutextrylock_syscall_helper(SyscallManager::Request& req) {
	return mutextrylock_syscall_handler((uint32_t)req.arg1);
}
static_assert(sizeof(uint32_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t vmcheckreadable_syscall_helper(SyscallManager::Request& req) {
	return vmcheckreadable_syscall_handler((uintptr_t)req.arg1,(size_t)req.arg2);
}
static_assert(sizeof(uintptr_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(size_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

syscall_response_t vmcheckwritable_syscall_helper(SyscallManager::Request& req) {
	return vmcheckwritable_syscall_handler((uintptr_t)req.arg1,(size_t)req.arg2);
}
static_assert(sizeof(uintptr_t) <= sizeof(uint32_t), "type is not safe to pass in a register");
static_assert(sizeof(size_t) <= sizeof(uint32_t), "type is not safe to pass in a register");

