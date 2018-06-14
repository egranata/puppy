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

#include <kernel/sys/stdint.h>
#include <kernel/synch/semaphore.h>
#include <kernel/libc/queue.h>
#include <kernel/libc/atomic.h>
#include <kernel/libc/fixstack.h>
#include <kernel/drivers/framebuffer/fb.h>

class TTY {
    public:
        TTY();
        void write(const char* buffer);
        int read();

        void pushfg(uint16_t);
        uint16_t popfg();

        void setPosition(uint16_t row, uint16_t col);

    private:
        static constexpr size_t gQueueSize = 1024;

        Semaphore mWriteSemaphore;
        FixSizeStack<uint16_t, 32> mForeground;
        queue<char, gQueueSize> mOutQueue;
        queue<char, gQueueSize> mInQueue;
        Framebuffer& mFramebuffer;
};

#endif
