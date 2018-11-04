/* <dirent.h> includes <sys/dirent.h>, which is this file.  On a
   system which supports <dirent.h>, this file is overridden by
   dirent.h in the libc/sys/.../sys directory.  On a system which does
   not support <dirent.h>, we will get this file which uses #error to force
   an error.  */

#ifdef __cplusplus
extern "C" {
#endif

#include <newlib/stdint.h>
#include <newlib/time.h>

#define DT_UNKNOWN           0
#define DT_PIPE              1
#define DT_CHR               2
#define DT_DIR               4
#define DT_BLK               6
#define DT_REG               8
#define DT_QUEUE            16
#define DT_TTY              32
#define DT_SEMAPHORE        64
#define DT_MUTEX           128

typedef struct dirent {
    int d_ino; // inode: required for compatibility but unused
    uint16_t d_reclen; // size of this record
    uint8_t d_type; // entry type
    uint32_t d_size; // size of the filesystem object
    time_t d_time; // time information associated to this entry
    char d_name[255 + 1];
} dirent;

typedef struct DIR {
    uint32_t fhnd;
    dirent current;
} DIR;

DIR* opendir(const char*);
int closedir(DIR*);
struct dirent* readdir(DIR*);
int readdir_r(DIR*, struct dirent*, struct dirent**);

#ifdef __cplusplus
}
#endif
