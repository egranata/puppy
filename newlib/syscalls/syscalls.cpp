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

#include <newlib/syscalls.h>

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

extern "C"
syscall_response_t syscall5(uint8_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

syscall_response_t yield_syscall() {
	return syscall0(yield_syscall_id);
}
syscall_response_t sleep_syscall(uint32_t arg1) {
	return syscall1(sleep_syscall_id,(uint32_t)arg1);
}
syscall_response_t exit_syscall(uint8_t arg1) {
	return syscall1(exit_syscall_id,(uint32_t)arg1);
}
syscall_response_t halt_syscall() {
	return syscall0(halt_syscall_id);
}
syscall_response_t reboot_syscall() {
	return syscall0(reboot_syscall_id);
}
syscall_response_t sysinfo_syscall(sysinfo_t* arg1,uint32_t arg2) {
	return syscall2(sysinfo_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t getcurdir_syscall(char* arg1,size_t* arg2) {
	return syscall2(getcurdir_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t setcurdir_syscall(const char* arg1) {
	return syscall1(setcurdir_syscall_id,(uint32_t)arg1);
}
syscall_response_t getpid_syscall() {
	return syscall0(getpid_syscall_id);
}
syscall_response_t fopen_syscall(const char* arg1,uint32_t arg2) {
	return syscall2(fopen_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t fclose_syscall(uint32_t arg1) {
	return syscall1(fclose_syscall_id,(uint32_t)arg1);
}
syscall_response_t fdup_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(fdup_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t fread_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3) {
	return syscall3(fread_syscall_id,(uint32_t)arg1,(uint32_t)arg2,(uint32_t)arg3);
}
syscall_response_t exec_syscall(const char* arg1,char** arg2,char** arg3,uint32_t arg4,exec_fileop_t* arg5) {
	return syscall5(exec_syscall_id,(uint32_t)arg1,(uint32_t)arg2,(uint32_t)arg3,(uint32_t)arg4,(uint32_t)arg5);
}
syscall_response_t kill_syscall(uint32_t arg1) {
	return syscall1(kill_syscall_id,(uint32_t)arg1);
}
syscall_response_t fstat_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(fstat_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t fseek_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(fseek_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t ftell_syscall(uint16_t arg1,size_t* arg2) {
	return syscall2(ftell_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t fopendir_syscall(uint32_t arg1) {
	return syscall1(fopendir_syscall_id,(uint32_t)arg1);
}
syscall_response_t freaddir_syscall(uint16_t arg1,file_info_t* arg2) {
	return syscall2(freaddir_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t getppid_syscall() {
	return syscall0(getppid_syscall_id);
}
syscall_response_t collect_syscall(kpid_t arg1,process_exit_status_t* arg2) {
	return syscall2(collect_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t fioctl_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3) {
	return syscall3(fioctl_syscall_id,(uint32_t)arg1,(uint32_t)arg2,(uint32_t)arg3);
}
syscall_response_t fwrite_syscall(uint32_t arg1,uint32_t arg2,uint32_t arg3) {
	return syscall3(fwrite_syscall_id,(uint32_t)arg1,(uint32_t)arg2,(uint32_t)arg3);
}
syscall_response_t prioritize_syscall(kpid_t arg1,prioritize_target arg2,const exec_priority_t* arg3,exec_priority_t* arg4) {
	return syscall4(prioritize_syscall_id,(uint32_t)arg1,(uint32_t)arg2,(uint32_t)arg3,(uint32_t)arg4);
}
syscall_response_t mapregion_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(mapregion_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t unmapregion_syscall(uint32_t arg1) {
	return syscall1(unmapregion_syscall_id,(uint32_t)arg1);
}
syscall_response_t setregionperms_syscall(uint32_t arg1,uint32_t arg2) {
	return syscall2(setregionperms_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t mount_syscall(uint32_t arg1,const char* arg2) {
	return syscall2(mount_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t unmount_syscall(const char* arg1) {
	return syscall1(unmount_syscall_id,(uint32_t)arg1);
}
syscall_response_t collectany_syscall(bool arg1,kpid_t* arg2,process_exit_status_t* arg3) {
	return syscall3(collectany_syscall_id,(uint32_t)arg1,(uint32_t)arg2,(uint32_t)arg3);
}
syscall_response_t clone_syscall(uintptr_t arg1,exec_fileop_t* arg2) {
	return syscall2(clone_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t fdel_syscall(const char* arg1) {
	return syscall1(fdel_syscall_id,(uint32_t)arg1);
}
syscall_response_t mkdir_syscall(const char* arg1) {
	return syscall1(mkdir_syscall_id,(uint32_t)arg1);
}
syscall_response_t proctable_syscall(process_info_t* arg1,size_t arg2) {
	return syscall2(proctable_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t vmcheckreadable_syscall(uintptr_t arg1,size_t arg2) {
	return syscall2(vmcheckreadable_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t vmcheckwritable_syscall(uintptr_t arg1,size_t arg2) {
	return syscall2(vmcheckwritable_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t pipe_syscall(size_t* arg1,size_t* arg2) {
	return syscall2(pipe_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t mmap_syscall(size_t arg1,int arg2) {
	return syscall2(mmap_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t wait1_syscall(uint16_t arg1,uint32_t arg2) {
	return syscall2(wait1_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t fsinfo_syscall(const char* arg1,filesystem_info_t* arg2) {
	return syscall2(fsinfo_syscall_id,(uint32_t)arg1,(uint32_t)arg2);
}
syscall_response_t checkfeatures_syscall(feature_id_t* arg1) {
	return syscall1(checkfeatures_syscall_id,(uint32_t)arg1);
}
