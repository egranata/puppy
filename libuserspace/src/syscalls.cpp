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

#include <syscalls.h>

extern "C"
syscall_response_t syscall0(uint8_t);

extern "C"
syscall_response_t syscall1(uint8_t, uint32_t);

extern "C"
syscall_response_t syscall2(uint8_t, uint32_t, uint32_t);

extern "C"
syscall_response_t syscall3(uint8_t, uint32_t, uint32_t, uint32_t);

extern "C"
syscall_response_t syscall4(uint8_t, uint32_t, uint32_t, uint32_t, uint32_t);

syscall_response_t yield_syscall() {
	return syscall0(yield_syscall_id);
}
syscall_response_t sleep_syscall(uint32_t arg1) {
	return syscall1(sleep_syscall_id,arg1);
}
syscall_response_t msgsend_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3) {
	return syscall3(msgsend_syscall_id,arg1,arg2,arg3);
}
syscall_response_t msgrecv_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(msgrecv_syscall_id,arg1,arg2);
}
syscall_response_t exit_syscall(uint32_t arg1) {
	return syscall1(exit_syscall_id,arg1);
}
syscall_response_t reboot_syscall() {
	return syscall0(reboot_syscall_id);
}
syscall_response_t semwait_syscall(uint32_t arg1) {
	return syscall1(semwait_syscall_id,arg1);
}
syscall_response_t semsignal_syscall(uint32_t arg1) {
	return syscall1(semsignal_syscall_id,arg1);
}
syscall_response_t getpid_syscall() {
	return syscall0(getpid_syscall_id);
}
syscall_response_t fopen_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(fopen_syscall_id,arg1,arg2);
}
syscall_response_t fclose_syscall(uint32_t arg1) {
	return syscall1(fclose_syscall_id,arg1);
}
syscall_response_t fread_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3) {
	return syscall3(fread_syscall_id,arg1,arg2,arg3);
}
syscall_response_t exec_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3) {
	return syscall3(exec_syscall_id,arg1,arg2,arg3);
}
syscall_response_t uptime_syscall(uint32_t arg1) {
	return syscall1(uptime_syscall_id,arg1);
}
syscall_response_t kill_syscall(uint32_t arg1) {
	return syscall1(kill_syscall_id,arg1);
}
syscall_response_t now_syscall(uint32_t arg1) {
	return syscall1(now_syscall_id,arg1);
}
syscall_response_t fstat_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(fstat_syscall_id,arg1,arg2);
}
syscall_response_t fseek_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(fseek_syscall_id,arg1,arg2);
}
syscall_response_t fopendir_syscall(uint32_t arg1) {
	return syscall1(fopendir_syscall_id,arg1);
}
syscall_response_t freaddir_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(freaddir_syscall_id,arg1,arg2);
}
syscall_response_t getppid_syscall() {
	return syscall0(getppid_syscall_id);
}
syscall_response_t collect_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(collect_syscall_id,arg1,arg2);
}
syscall_response_t semget_syscall(uint32_t arg1) {
	return syscall1(semget_syscall_id,arg1);
}
syscall_response_t fioctl_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3) {
	return syscall3(fioctl_syscall_id,arg1,arg2,arg3);
}
syscall_response_t fwrite_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3) {
	return syscall3(fwrite_syscall_id,arg1,arg2,arg3);
}
syscall_response_t prioritize_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(prioritize_syscall_id,arg1,arg2);
}
syscall_response_t mutexget_syscall(uint32_t arg1) {
	return syscall1(mutexget_syscall_id,arg1);
}
syscall_response_t mutexlock_syscall(uint32_t arg1) {
	return syscall1(mutexlock_syscall_id,arg1);
}
syscall_response_t mutexunlock_syscall(uint32_t arg1) {
	return syscall1(mutexunlock_syscall_id,arg1);
}
syscall_response_t mapregion_syscall(uint32_t arg1) {
	return syscall1(mapregion_syscall_id,arg1);
}
syscall_response_t getcontroller_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(getcontroller_syscall_id,arg1,arg2);
}
syscall_response_t discoverdisk_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(discoverdisk_syscall_id,arg1,arg2);
}
syscall_response_t writesector_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3,uint32_t arg4) {
	return syscall4(writesector_syscall_id,arg1,arg2,arg3,arg4);
}
syscall_response_t readsector_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3,uint32_t arg4) {
	return syscall4(readsector_syscall_id,arg1,arg2,arg3,arg4);
}
syscall_response_t getmeminfo_syscall(uint32_t arg1) {
	return syscall1(getmeminfo_syscall_id,arg1);
}
