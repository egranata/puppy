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

// This file should be includable from both kernel and userspace - it is meant to contain common macros and types for the kernel <--> userspace interface
#ifndef SYSCALLS_TYPES
#define SYSCALLS_TYPES

#include <kernel/sys/stdint.h>
using syscall_response_t = uint32_t;

struct sysinfo_t {
    struct {
        uint64_t uptime; /** uptime of the system */
        uint32_t totalmem; /** total amount of RAM */
        uint32_t freemem; /** amount of free RAM */
        uint64_t ctxswitches; /** number of context switches since boot */
    } global;
    struct {
        uint64_t runtime; /** time this process has been running */
        uint32_t allocated; /** amount of memory allocated to this process */
        uint32_t committed; /** amount of actual RAM given to this process */
        uint32_t pagefaults; /** number of page faults that this process caused */
    } local;
};

enum {
    INCLUDE_GLOBAL_INFO = 1,
    INCLUDE_LOCAL_INFO = 2,
};

enum {
    FILE_OPEN_READ = 1, // allow reading
    FILE_OPEN_WRITE = 2, // allow writing
    FILE_OPEN_NEW = 4, // if the file exists, create a new one anyway
    FILE_NO_CREATE = 8, // if the file does not exist, do not create it
    FILE_OPEN_APPEND = 16 // append to the file, if it exists and has content
};

enum {
    REGION_ALLOW_WRITE = 1,
};

enum {
    PROCESS_IS_FOREGROUND = 0x1, // the new process will acquire foreground on the tty
    PROCESS_INHERITS_CWD  = 0x2, // the new process will share the parent's CWD (if not set, defaults to "/")
    PROCESS_NOT_VALID_FLAG0 = 0x4,  // this is used by newlib to mean "do not inherit environment variables"
};

struct process_exit_status_t {
    enum class reason_t : uint8_t {
        alive = 0, /** this process is still alive */
        cleanExit = 1, /** this process chose to exit of its own free will */
        exception = 2, /** this process caused a fatal exception */
        killed = 3, /** this process was killed */
        kernelError = 0xFF /** the kernel encountered an issue and terminated the process */
    } reason;
    uint8_t status; /** exit code if clean-exit, error code if exception, reserved otherwise */

    uint32_t toWord() {
        return status | (((uint8_t)reason) << 24);
    }

    process_exit_status_t(reason_t r, uint8_t s) : reason(r), status(s) {}

    explicit process_exit_status_t(uint32_t word) {
        status = word & 0xFF;
        reason = (reason_t)(word >> 24);
    }
};

struct blockdevice_usage_stats_t {
    uint64_t sectors_read;
    uint64_t cache_hits;
    uint64_t sectors_written;
};

// IOCTL operations that one can run on a block device file
enum class blockdevice_ioctl_t : uintptr_t {
    IOCTL_GET_SECTOR_SIZE = 0xB10C0001,   // (a=IOCTL_, b=0), returns sector size
    IOCTL_GET_NUM_SECTORS = 0xB10C0002,   // (a=IOCTL_, b=0), returns number of sectors
    IOCTL_GET_CONTROLLER  = 0xB10C0003,   // (a=IOCTL_, b=0), returns an opaque disk controller descriptor
    IOCTL_GET_ROUTING     = 0xB10C0004,   // (a=IOCTL_, b=0), returns an opaque routing descriptor
    IOCTL_GET_VOLUME      = 0xB10C0005,   // (a=IOCTL_, b=0), returns a Volume* suitable for mounting
    IOCTL_GET_USAGE_STATS = 0xB10C0006,   // (a=IOCTL_, b=pointer to blockdevice_usage_stats_t*), returns 1 if success and data is filled in; 0 otherwise
};

// IOCTL operations that one can run on a TTY
enum tty_ioctl_t {
        IOCTL_FOREGROUND            = 0x77100001, // a2 = pid
        IOCTL_BACKGROUND            = 0x77100002, // a2 = reserved
        IOCTL_MOVECURSOR            = 0x77100003, // a2 = 0xRRRRCCCC
        IOCTL_VISIBLE_AREA          = 0x77100004, // a2 = pointer to 0xRRRRCCCC
        IOCTL_CURSOR_POS            = 0x77100005, // a2 = pointer to 0xRRRRCCCC
        IOCTL_GET_FG_COLOR          = 0x77100006, // a2 = pointer to 0x00RRGGBB
        IOCTL_SET_FG_COLOR          = 0x77100007, // a2 = 0x00RRGGBB
        IOCTL_GET_BG_COLOR          = 0x77100008, // a2 = pointer to 0x00RRGGBB
        IOCTL_SET_BG_COLOR          = 0x77100009, // a2 = 0x00RRGGBB
        IOCTL_DISCIPLINE_RAW        = 0x7710000A, // a2 = ignored
        IOCTL_DISCIPLINE_CANONICAL  = 0x7710000B, // a2 = ignored
        IOCTL_DISCIPLINE_GET        = 0x7710000C, // a2 = pointer to int - on return contains 10 for 'RAW', 11 for 'CANONICAL'
};

// IOCTL operations that can run on a msgqueue
enum msgqueue_ioctl_t {
    IOCTL_GET_QUEUE_SIZE = 1, // a2 = reserved, return size of queue
    IOCTL_BLOCK_ON_EMPTY = 2, // a2 = bool; should a read block if the queue is empty
    IOCTL_BLOCK_ON_FULL = 3, // a2 = bool; should a write block if the queue is full
};

// IOCTL operations that can run on a semaphore
enum semaphore_ioctl_t {
    IOCTL_SEMAPHORE_WAIT   = 0x5EA01111,
    IOCTL_SEMAPHORE_SIGNAL = 0x5EA02222,
};

enum class process_state_t : uint8_t {
    NEW, /** created and not schedulable */
    AVAILABLE, /** ready to be scheduled (or running, we don't distinguish yet) */
    WAITING, /** in a waitqueue */
    SLEEPING, /** waiting to be woken up at sleeptill */
    EXITED, /** waiting for parent to collect */
    COLLECTED, /** waiting to be torn down by the system */
    COLLECTING, /** waiting to collect a child */
};

static constexpr size_t gMaxPathSize = 255;
typedef char path_name_t[gMaxPathSize + 1];

enum class file_kind_t {
#define DIR_LIKE(x,y) x = y,
#define FILE_LIKE(x,y) x = y,
#include <kernel/fs/file_kinds.tbl>
#undef DIR_LIKE
#undef FILE_LIKE
};

struct file_stat_t {
    file_kind_t kind;
    uint32_t size;
    uint64_t time;
};

struct file_info_t {
    using kind_t = file_kind_t;
    uint32_t size;
    uint64_t time;
    path_name_t name;
    file_kind_t kind;
};

// make sure that userspace can define its own pid_t with different requirements than our version here
typedef uint16_t kpid_t;

struct process_info_t {
    static constexpr size_t gMaxCommandLineSize = 128;

    kpid_t pid;
    kpid_t ppid;
    process_state_t state;
    char path[gMaxPathSize];
    char args[gMaxCommandLineSize];

    uintptr_t vmspace;
    uintptr_t pmspace;
    uint64_t runtime;

    uint64_t diskReadBytes;
    uint64_t diskWrittenBytes;

#define FLAG_PUBLIC(name, bit) bool name;
#define FLAG_PRIVATE(name, bit)
    struct {
#include <kernel/process/flags.tbl>
    } flags;
};

struct klog_stats_t {
    uint64_t numentries;
    uint64_t totalwritten;
    size_t   largestentry;
};

enum class prioritize_target : bool {
    PRIORITIZE_SET_CURRENT = false,
    PRIORITIZE_SET_MAXIMUM = true
};

#ifndef TIOCGWINSZ
static constexpr uint32_t TIOCGWINSZ = 0x40087468;
#endif
struct winsize_t { // the return of TIOCGWINSZ
  unsigned short ws_row;	/* rows, in characters */
  unsigned short ws_col;	/* columns, in characters */
  unsigned short ws_xpixel;	/* horizontal size, pixels */
  unsigned short ws_ypixel;	/* vertical size, pixels */
};

struct exec_priority_t {
    uint8_t quantum; /* how many clock ticks a process can run once scheduled */
    uint64_t scheduling; /* how likely a process is to be scheduled */
};

struct exec_fileop_t {
    enum class operation : uint8_t {
        END_OF_LIST = 0, // all param values ignored; stop parsing
        CLOSE_CHILD_FD, // param1, the file descriptor to close
        DUP_PARENT_FD, // param1, the file descriptor to fdup - on return, param2 contains the descriptor in the child
        DUP_CHILD_FD, // param1, the file descriptor to fdup
    };

    operation op;
    size_t  param1;
    size_t  param2; // reserved for future evolutions
    void*     param3; // reserved for future evolutions
};

struct message_t {
    static constexpr size_t gTotalSize = 4096; // TODO: expose the size of a page globally
    struct header_t {
        kpid_t sender;
        uint64_t timestamp;
        size_t payload_size;
    } header;
    static constexpr size_t gBodySize = gTotalSize - sizeof(header_t);
    uint8_t payload[gBodySize];
};

static_assert(sizeof(message_t) == message_t::gTotalSize);

#endif
