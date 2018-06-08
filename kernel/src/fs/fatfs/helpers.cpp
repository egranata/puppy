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

#include <fatfs/include/ff.h>
#include <fs/fatfs/fs.h>
#include <fs/vol/volume.h>

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
        case GET_SECTOR_SIZE: *(uint32_t*)buff = 512; break;
        case GET_BLOCK_SIZE: *(uint32_t*)buff = 1; break;
        default: return RES_PARERR;
    }

    return RES_OK;
}
