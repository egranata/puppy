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

#ifndef SYSCALLS_MANAGER
#define SYSCALLS_MANAGER

#include <sys/stdint.h>
#include <sys/nocopy.h>

typedef uint32_t syscall_response_t;

#define DEFINE_ERROR(name, value) \
    static constexpr uint32_t SYSCALL_ERR_ ## name = (value << 1) | SYSCALL_ERROR

class SyscallManager : NOCOPY {
    public:
        static constexpr uint32_t SYSCALL_SUCCESS = 0;
        static constexpr uint32_t SYSCALL_ERROR = 1;

        static constexpr uint8_t gSyscallIRQ = 0x80;

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

        struct Request {
            uint8_t code;
            uint32_t arg1;
            uint32_t arg2;
            uint32_t arg3;
            uint32_t arg4;

            uintptr_t eip;
        };
        using Handler = syscall_response_t(*)(Request&);

        static SyscallManager& get();

    private:
        SyscallManager();

        void sethandlers();

        void handle(uint8_t code, Handler handler, bool systemOnly);
};

#undef DEFINE_ERROR

#endif
