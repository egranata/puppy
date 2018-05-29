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

#ifndef TTY_FILE
#define TTY_FILE

#include <fs/filesystem.h>
#include <tty/tty.h>

class TTYFile : public Filesystem::File {
    public:
        static constexpr uint32_t IOCTL_FOREGROUND = 1; // a2 = pid
        static constexpr uint32_t IOCTL_BACKGROUND = 2; // a2 = reserved
        static constexpr uint32_t IOCTL_MOVECURSOR = 3; // a2 = low = row, high = col

        TTYFile(TTY* tty);

        bool seek(size_t) override;
        bool read(size_t, char*) override;
        bool write(size_t, char*) override;
        bool stat(stat_t&) override;
        uintptr_t ioctl(uintptr_t, uintptr_t) override;
    private:
        TTY* mTTY;
};

#endif
