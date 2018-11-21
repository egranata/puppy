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

#ifndef TTY_TTY
#define TTY_TTY

#include <stdint.h>
#include <kernel/synch/semaphore.h>
#include <kernel/synch/waitqueue.h>
#include <kernel/libc/queue.h>
#include <kernel/libc/atomic.h>
#include <kernel/libc/vec.h>
#include <kernel/drivers/framebuffer/fb.h>
#include <kernel/drivers/ps2/keyboard.h>
#include <kernel/syscalls/types.h>
#include <kernel/tty/keyevent.h>

class TTY {
    public:
        static constexpr int TTY_NO_INPUT = -1; // nothing is currently available to be read
        static constexpr int TTY_EOF_MARKER = -2; // the TTY has received an end-of-file marking

        TTY();
        void write(size_t sz, const char* buffer);
        key_event_t readKeyEvent();

        void pushfg(kpid_t);
        kpid_t popfg();

        void setPosition(uint16_t row, uint16_t col);
        void getPosition(uint16_t *row, uint16_t* col);

        void getSize(uint16_t *rows, uint16_t* cols);

        void getCurrentForegroundColor(uint32_t *color);
        void getCurrentBackgroundColor(uint32_t *color);

        void setCurrentForegroundColor(uint32_t color);
        void setCurrentBackgroundColor(uint32_t color);

        void getConfiguredForegroundColor(uint32_t *color);
        void getConfiguredBackgroundColor(uint32_t *color);

        void setConfiguredForegroundColor(uint32_t color);
        void setConfiguredBackgroundColor(uint32_t color);

        void clearLine(bool from_cursor, bool to_cursor);
        void clearScreen();

        void killForegroundProcess();

        void resetGraphics();
        void swapColors();
        void setANSIBackgroundColor(int code);
        void setANSIForegroundColor(int code);

        void setANSIBackgroundColor(int r, int g, int b);
        void setANSIForegroundColor(int r, int g, int b);

    private:
        static constexpr size_t gQueueSize = 1024;

        Semaphore mWriteSemaphore;
        vector<kpid_t> mForeground;
        queue<char, gQueueSize> mOutQueue;
        queue<char, gQueueSize> mInQueue;
        Framebuffer& mFramebuffer;
        WaitQueue mForegroundWQ;
};

#endif
