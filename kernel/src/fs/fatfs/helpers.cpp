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

#include <fatfs/ff.h>

#include <kernel/time/manager.h>
#include <kernel/libc/time.h>
#include <kernel/fs/fatfs/fs.h>
#include <kernel/fs/vol/volume.h>

namespace {
    union fat_time_t {
        uint32_t value;
        struct {
            uint8_t second : 5;
            uint8_t minute : 6;
            uint8_t hour : 5;
            uint8_t day : 5;
            uint8_t month : 4;
            uint8_t year : 7;
        } __attribute__((packed)) representation;
    } __attribute__((packed));

    static_assert(sizeof(fat_time_t) == sizeof(uint32_t));
    // wish I could write this:
    // static_assert( fat_time_t{1}.representation.second == 1 );
}

extern "C"
uint32_t get_fattime() {
    date_components_t date;
    time_components_t time;
    fat_time_t ftt;

    epoch_to_time_and_date_components(TimeManager::get().UNIXtime(), date, time);
    ftt.representation.second = time.second >> 1;
    ftt.representation.minute = time.minute;
    ftt.representation.hour = time.hour;

    ftt.representation.day = date.day;
    ftt.representation.month = date.month;
    ftt.representation.year = date.year - 1980;

    return ftt.value;
}

extern "C"
DSTATUS disk_initialize (FATFS*) {
    return RES_OK;
}

extern "C"
DSTATUS disk_status (FATFS*) {
    return RES_OK;
}

extern "C"
DRESULT disk_read (FATFS* pdrv, BYTE* buff, DWORD sector, UINT count) {
    return pdrv->vol->read(sector, count, buff) ? RES_OK : RES_ERROR;
}

extern "C"
DRESULT disk_write (FATFS* pdrv, const BYTE* buff, DWORD sector, UINT count) {
    return pdrv->vol->write(sector, count, (unsigned char*)buff) ? RES_OK : RES_ERROR;
}

extern "C"
DRESULT disk_ioctl (FATFS* pdrv, BYTE cmd, void* buff) {
    switch (cmd) {
        case CTRL_SYNC:
        case CTRL_TRIM: break;
        case GET_SECTOR_COUNT: *(uint32_t*)buff = pdrv->vol->numsectors(); break;
        case GET_SECTOR_SIZE: *(uint32_t*)buff = pdrv->vol->sectorsize(); break;
        case GET_BLOCK_SIZE: *(uint32_t*)buff = 1; break;
        default: return RES_PARERR;
    }

    return RES_OK;
}
