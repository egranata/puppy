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

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>

typedef uint32_t syscall_response_t;

// this should only be defined inside the class, but then syscall_error/action_code
// couldn't be defined because reasons (see https://stackoverflow.com/questions/29551223/static-constexpr-function-called-in-a-constant-expression-is-an-error
// for details) - so duplicate this globally
static constexpr uint32_t SYSCALL_ERROR = 1;

static constexpr uint8_t gNumActionBits = 4;
static constexpr uint32_t syscall_error_code(uint16_t code) {
    return (code << (gNumActionBits + 1)) | SYSCALL_ERROR;
}
static constexpr uint32_t syscall_action_code(uint8_t code) {
    return (1 << (code+1)) | SYSCALL_ERROR;
}

class SyscallManager : NOCOPY {
    public:
        static constexpr uint32_t SYSCALL_SUCCESS = 0;
        static constexpr uint32_t SYSCALL_ERROR = ::SYSCALL_ERROR;

        static constexpr uint8_t gNumActionBits = ::gNumActionBits;

#define DEFINE_ERROR(name, value) \
    static constexpr uint32_t SYSCALL_ERROR_ ## name = syscall_error_code(value)

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

#define DEFINE_ACTION(name, value) \
    static_assert(value < gNumActionBits); \
    static constexpr uint32_t SYSCALL_ACTION_ ## name = syscall_action_code(value)

        static constexpr uint8_t gSyscallIRQ = 0x80;

        struct Request {
            uint8_t code;
            uint32_t arg1;
            uint32_t arg2;
            uint32_t arg3;
            uint32_t arg4;
            uint32_t arg5;

            uint32_t eflags;
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
#undef DEFINE_ACTION

#endif
