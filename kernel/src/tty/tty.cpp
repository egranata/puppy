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

#include <kernel/tty/tty.h>
#include <kernel/drivers/ps2/controller.h>
#include <kernel/process/manager.h>
#include <kernel/log/log.h>
#include <kernel/tasks/keybqueue.h>

TTY::TTY() : mWriteSemaphore("tty", 1, 1), mForeground(), mOutQueue(), mInQueue(), mFramebuffer(Framebuffer::get()) {}

void TTY::write(size_t sz, const char* buffer) {
    mWriteSemaphore.wait();

    while(sz) {
        mFramebuffer.putc(*buffer);
        --sz;
        ++buffer;
    }

    mWriteSemaphore.signal();
}

void TTY::pushfg(kpid_t pid) {
    LOG_DEBUG("tty foreground given to %u", pid);
    mForeground.push_back(pid);
    mForegroundWQ.wakeall();
}

kpid_t TTY::popfg(kpid_t pid) {
    kpid_t newfg = 0;
    class WakeOnExit {
        public:
            WakeOnExit(WaitQueue* wq) : mWQ(wq) {}
            ~WakeOnExit() {
                    mWQ->wakeall();
            }
        private:
            WaitQueue* mWQ;
    } wake(&mForegroundWQ);

    auto&& pmm(ProcessManager::get());

    LOG_DEBUG("pid %u out of tty foreground", pid);
    if (mForeground.empty()) goto init_failsafe;
    mForeground.eraseAll(pid);
    if (mForeground.empty()) goto init_failsafe;
    newfg = mForeground.back();
    LOG_DEBUG("tty foreground goes to %u", newfg);
    return newfg;

init_failsafe:
    LOG_DEBUG("no live process in chain, tty goes back to init");
    return mForeground.push_back(pmm.initpid()), mForeground.back();
}

key_event_t TTY::readKeyEvent() {
    bool allow = false;
    
    do {
        if (mForeground.empty()) {
            allow = true;
        } else {
            allow = (mForeground.back() == gCurrentProcess->pid);
        }

        if (false == allow) {
            LOG_WARNING("process %u wanting to use TTY but is not foreground", gCurrentProcess->pid);
            mForegroundWQ.wait(gCurrentProcess);
        }
    } while(false == allow);

    auto evt = tasks::keybqueue::readKey();
    if (evt.down || evt.keycode == 0) return key_event_t();
    return evt;
}

void TTY::setPosition(uint16_t row, uint16_t col) {
    mFramebuffer.setRow(row);
    mFramebuffer.setCol(col);
}

void TTY::getPosition(uint16_t *row, uint16_t* col) {
    *row = mFramebuffer.row();
    *col = mFramebuffer.column();
}

void TTY::getSize(uint16_t *rows, uint16_t* cols) {
    *rows = mFramebuffer.rows();
    *cols = mFramebuffer.columns();
}

void TTY::getCurrentForegroundColor(uint32_t *color) {
    *color = (uint32_t)mFramebuffer.getForegroundColor(Framebuffer::CURRENT_COLOR_SET);
}

void TTY::getCurrentBackgroundColor(uint32_t *color) {
    *color = (uint32_t)mFramebuffer.getBackgroundColor(Framebuffer::CURRENT_COLOR_SET);
}

void TTY::setCurrentForegroundColor(uint32_t color) {
    mFramebuffer.setForegroundColor(Framebuffer::CURRENT_COLOR_SET, Framebuffer::color_t(color));
}

void TTY::setCurrentBackgroundColor(uint32_t color) {
    mFramebuffer.setBackgroundColor(Framebuffer::CURRENT_COLOR_SET, Framebuffer::color_t(color));
}

void TTY::getConfiguredForegroundColor(uint32_t *color) {
    *color = (uint32_t)mFramebuffer.getForegroundColor(Framebuffer::CONFIGURED_COLOR_SET);
}

void TTY::getConfiguredBackgroundColor(uint32_t *color) {
    *color = (uint32_t)mFramebuffer.getBackgroundColor(Framebuffer::CONFIGURED_COLOR_SET);
}

void TTY::setConfiguredForegroundColor(uint32_t color) {
    mFramebuffer.setForegroundColor(Framebuffer::CONFIGURED_COLOR_SET, Framebuffer::color_t(color));
}

void TTY::setConfiguredBackgroundColor(uint32_t color) {
    mFramebuffer.setBackgroundColor(Framebuffer::CONFIGURED_COLOR_SET, Framebuffer::color_t(color));
}

void TTY::clearLine(bool from_cursor, bool to_cursor) {
    mFramebuffer.clearLine(from_cursor, to_cursor);
}

void TTY::clearScreen() {
    mFramebuffer.cls();
}

void TTY::killForegroundProcess() {
    if (!mForeground.empty()) {
        auto pid = mForeground.back();
        ProcessManager::get().kill(pid);
    }
}

void TTY::resetGraphics() {
    mFramebuffer.setForegroundColor(Framebuffer::CURRENT_COLOR_SET,
        mFramebuffer.getForegroundColor(Framebuffer::CONFIGURED_COLOR_SET));
    mFramebuffer.setBackgroundColor(Framebuffer::CURRENT_COLOR_SET,
        mFramebuffer.getBackgroundColor(Framebuffer::CONFIGURED_COLOR_SET));
}

void TTY::swapColors() {
    uint32_t bg, fg;
    getCurrentBackgroundColor(&bg);
    getCurrentForegroundColor(&fg);

    setCurrentForegroundColor(bg);
    setCurrentBackgroundColor(fg);
}

static Framebuffer::color_t gANSIColors[] = {
/* 30-40 */    Framebuffer::color_t(0,   0,   0),
/* 31-41 */    Framebuffer::color_t(255, 0,   0),
/* 32-42 */    Framebuffer::color_t(0,   170, 0),
/* 33-43 */    Framebuffer::color_t(187, 187, 0),
/* 34-44 */    Framebuffer::color_t(0,   0,   187),
/* 35-45 */    Framebuffer::color_t(170, 0,   170),
/* 36-46 */    Framebuffer::color_t(0,   170, 170),
/* 37-47 */    Framebuffer::color_t(255, 255, 255),
};

void TTY::setANSIBackgroundColor(int code) {
    if (code < 40 || code > 49) return;

    // TODO: support custom color code
    if (code == 48) return;

    if (code == 49) {
        mFramebuffer.setBackgroundColor(Framebuffer::CURRENT_COLOR_SET,
            mFramebuffer.getBackgroundColor(Framebuffer::DEFAULT_COLOR_SET));
    } else {
        code -= 40;
        mFramebuffer.setBackgroundColor(Framebuffer::CURRENT_COLOR_SET, gANSIColors[code]);
    }
}

void TTY::setANSIForegroundColor(int code) {
    if (code < 30 || code > 39) return;

    // TODO: support custom color code
    if (code == 38) return;

    if (code == 39) {
        mFramebuffer.setForegroundColor(Framebuffer::CURRENT_COLOR_SET,
            mFramebuffer.getForegroundColor(Framebuffer::DEFAULT_COLOR_SET));
    } else {
        code -= 30;
        mFramebuffer.setForegroundColor(Framebuffer::CURRENT_COLOR_SET, gANSIColors[code]);
    }
}

void TTY::setANSIBackgroundColor(int r, int g, int b) {
    mFramebuffer.setBackgroundColor(Framebuffer::CURRENT_COLOR_SET,
        Framebuffer::color_t(r, g, b));
}
void TTY::setANSIForegroundColor(int r, int g, int b) {
    mFramebuffer.setForegroundColor(Framebuffer::CURRENT_COLOR_SET,
        Framebuffer::color_t(r, g, b));
}
