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
    mForeground.push(pid);
    mForegroundWQ.wakeall();
}

kpid_t TTY::popfg() {
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

    if (mForeground.empty()) return 0;
    if (mForeground.peek() != gCurrentProcess->pid) return mForeground.peek();

    while (!mForeground.empty()) {
        mForeground.pop();
        auto n = mForeground.peek();
        auto p = pmm.getprocess(n);
        if (p == nullptr || p->state == process_t::State::EXITED)
            continue;
        else {
            LOG_DEBUG("tty foreground given to %u", n);
            return n;
        }
    }
    
    LOG_DEBUG("no live process in chain, tty goes back to init");
    return mForeground.push(pmm.initpid()), mForeground.peek();
}

key_event_t TTY::readKeyEvent() {
    bool allow = false;
    
    do {
        if (mForeground.empty()) {
            allow = true;
        } else {
            allow = (mForeground.peek() == gCurrentProcess->pid);
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

void TTY::getForegroundColor(uint32_t *color) {
    *color = (uint32_t)mFramebuffer.getfg();
}

void TTY::getBackgroundColor(uint32_t *color) {
    *color = (uint32_t)mFramebuffer.getbg();
}

void TTY::setForegroundColor(uint32_t color) {
    mFramebuffer.setfg(Framebuffer::color_t(color));
}

void TTY::setBackgroundColor(uint32_t color) {
    const bool recolor = true;
    mFramebuffer.setbg(Framebuffer::color_t(color), recolor);
}

void TTY::clearLine(bool from_cursor, bool to_cursor) {
    mFramebuffer.clearLine(from_cursor, to_cursor);
}

void TTY::clearScreen() {
    mFramebuffer.cls();
}

void TTY::killForegroundProcess() {
    if (!mForeground.empty()) {
        auto pid = mForeground.peek();
        ProcessManager::get().kill(pid);
    }
}

void TTY::resetGraphics() {
    mFramebuffer.setbg(Framebuffer::defaultBackgroundColor, true);
    mFramebuffer.setfg(Framebuffer::defaultForegroundColor);
}

void TTY::swapColors() {
    auto bg = mFramebuffer.getbg();
    auto fg = mFramebuffer.getfg();

    mFramebuffer.setbg(fg, true);
    mFramebuffer.setfg(bg);
}

static Framebuffer::color_t gANSIColors[] = {
/* 30 */    Framebuffer::color_t(0,   0,   0),
/* 31 */    Framebuffer::color_t(255, 0,   0),
/* 32 */    Framebuffer::color_t(0,   170, 0),
/* 33 */    Framebuffer::color_t(187, 187, 0),
/* 34 */    Framebuffer::color_t(0,   0,   187),
/* 35 */    Framebuffer::color_t(170, 0,   170),
/* 36 */    Framebuffer::color_t(0,   170, 170),
/* 37 */    Framebuffer::color_t(255, 255, 255),
};

void TTY::setANSIBackgroundColor(int code) {
    if (code < 30 || code > 39) return;

    // TODO: support custom color code
    if (code == 38) return;

    if (code == 39) {
        mFramebuffer.setbg(Framebuffer::defaultBackgroundColor, true);
    } else {
        code -= 30;
        mFramebuffer.setbg(gANSIColors[code], true);
    }
}

void TTY::setANSIForegroundColor(int code) {
    if (code < 30 || code > 39) return;

    // TODO: support custom color code
    if (code == 38) return;

    if (code == 39) {
        mFramebuffer.setfg(Framebuffer::defaultForegroundColor);
    } else {
        code -= 30;
        mFramebuffer.setfg(gANSIColors[code]);
    }
}

void TTY::setANSIBackgroundColor(int r, int g, int b) {
    mFramebuffer.setbg(Framebuffer::color_t(r, g, b), true);
}
void TTY::setANSIForegroundColor(int r, int g, int b) {
    mFramebuffer.setfg(Framebuffer::color_t(r, g, b));
}
