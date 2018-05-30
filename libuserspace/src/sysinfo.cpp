#include <syscalls.h>
#include <sysinfo.h>

sysinfo_t sysinfo(bool global, bool local) {
    sysinfo_t si;

    if (0 == sysinfo_syscall(
        &si,
        (global ? INCLUDE_GLOBAL_INFO : 0) |
        (local ? INCLUDE_LOCAL_INFO : 0)
    )) {
        return si;
    } else {
        return sysinfo_t();
    }
}
