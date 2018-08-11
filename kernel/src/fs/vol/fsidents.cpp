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

#include <kernel/fs/fsidents.h>
#include <kernel/fs/fatfs/trymount.h>

fs_ident_t gKnownFilesystemTypes[] = {
    {0x00, "empty", nullptr},
    {0x01, "MS-DOS 12bit", nullptr},
    {0x04, "MS-DOS 16bit <32M", fatfs_trymount},
    {0x05, "DOS extended", nullptr},
    {0x06, "MS-DOS 16bit > 32M", fatfs_trymount},
    {0x07, "NTFS", nullptr},
    {0x0B, "Windows 95 FAT32 < 2GB", fatfs_trymount},
    {0x0C, "Windows 95 FAT32 LBA", fatfs_trymount},
    {0x0E, "Windows 95 16bit LBA", nullptr},
    {0x0F, "Windows 95 extended LBA", nullptr},
    {0x3C, "PartitionMagic", nullptr},
    {0x42, "Windows 2000 dynamic disk", nullptr},
    {0x82, "Solaris / Linux swap", nullptr},
    {0x83, "Linux native", nullptr},
    {0x84, "APM hibernation snapshot", nullptr},
    {0x8E, "Linux logical volume", nullptr},
    {0xA8, "Darwin x86", nullptr},
    {0xAB, "macOS boot", nullptr},
    {0xAF, "macOS HFS+", nullptr},
    {0xEE, "EFI header follows", nullptr},
    {0xEF, "EFI partition", nullptr},
    {0xFA, "Bochs x86 emulator", nullptr},
    {0xFB, "VMWare file system", nullptr},
    {0xFC, "VMWare swap", nullptr},
    {0xFF, nullptr, nullptr}
};
