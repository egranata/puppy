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

#include <kernel/fs/filesystem.h>
#include <kernel/tty/tty.h>

class TTYFile : public Filesystem::File {
    public:
        TTYFile(TTY* tty);

        bool seek(size_t) override;
        size_t read(size_t, char*) override;
        size_t write(size_t, char*) override;
        bool stat(stat_t&) override;
        uintptr_t ioctl(uintptr_t, uintptr_t) override;
    private:
        TTY* mTTY;
        struct input_t {
            char buf[1024];
            size_t mNextWrite;
            size_t mNextRead;

            bool appendOne(char c);
            bool undoAppend(char* c);
            bool consumeOne(char* c);
            bool emptyWrite() const;
            bool emptyRead() const;
            void clear();
        } mInput;
        enum class mode_t {
            READ_FROM_IRQ = 0xF00D,
            CONSUME_BUFFER = 0xBEEF,
        } mMode = mode_t::READ_FROM_IRQ;

        char procureOne();
};

#endif
