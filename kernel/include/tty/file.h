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

        void setTTY(TTY* tty);

        bool seek(size_t) override;
        size_t read(size_t, char*) override;
        size_t write(size_t, char*) override;
        bool doStat(stat_t&) override;
        uintptr_t ioctl(uintptr_t, uintptr_t) override;
    private:
        TTY* mTTY;
        struct input_t {
            static constexpr char EOF_MARKER = 0xFF;

            char buf[1024];
            size_t mNextWrite;
            size_t mNextRead;

            bool appendOne(char c);
            bool undoAppend(char* c);
            bool consumeOne(char* c);
            int peekOne();
            bool emptyWrite() const;
            bool emptyRead() const;
            void clear();
        } mInput;
        enum class mode_t {
            READ_FROM_IRQ = 0xF00D,
            CONSUME_BUFFER = 0xBEEF,
            ONLY_EOF = 0xE00F,
        } mMode = mode_t::READ_FROM_IRQ;

        enum class discipline_t : int {
            RAW = 10,
            CANONICAL = 11,
        } mDiscipline = discipline_t::CANONICAL;

        // TODO: support more than just basic one input CSI
        enum class escape_sequence_status_t {
            OFF,
            ESC,
            CSI
        } mEscapeStatus = escape_sequence_status_t::OFF;
        int mEscapeSequenceInput = 0;

        key_event_t procureOne();

        void processOne_Canonical(key_event_t ch);
        void processOne_Raw(key_event_t ch);

        // true means the chord is handled, false means bubble it upwards
        bool interceptChords_Canonical(const key_event_t&, bool* eofHappened);
        bool interceptChords_Raw(const key_event_t&, bool* eofHappened);

};

#endif
