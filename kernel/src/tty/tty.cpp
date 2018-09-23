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

TTY::TTY() : mWriteSemaphore("tty", 1, 1), mForeground(), mOutQueue(), mInQueue(), mFramebuffer(Framebuffer::get()), mEOF(false) {}

void TTY::write(size_t sz, const char* buffer) {
    mWriteSemaphore.wait();

    while(sz) {
        mFramebuffer.putc(*buffer);
        --sz;
        ++buffer;
    }

    mWriteSemaphore.signal();
}

void TTY::pushfg(uint16_t pid) {
    LOG_DEBUG("tty foreground given to %u", pid);
    mForeground.push(pid);
}

uint16_t TTY::popfg() {
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

void TTY::resetEOF() {
    mEOF = false;
}

#define CTRL evt.ctrldown
#define ALT evt.altdown
#define KEY(x) (evt.keycode == x)

bool TTY::interceptChords(const PS2Keyboard::key_event_t& evt) {
    if (evt.down) return false;
    if (CTRL && ALT && (KEY('K') || KEY('k'))) {
        mFramebuffer.cls();
        return true;
    }

    if (CTRL && (KEY('C') || KEY('c'))) {
        if (!mForeground.empty()) {
            auto pid = mForeground.peek();
            ProcessManager::get().kill(pid);
            return true;
        }
    }

    if (CTRL && (KEY('D') || KEY('d'))) {
        if (!mForeground.empty()) {
            mEOF = true;
            return true;
        }
    }
    return false;
}

#undef CTRL
#undef ALT
#undef KEY

int TTY::read() {
    // TODO: TTY spawns a kernel thread that reads and buffers keyboard events    

    auto&& pmm(ProcessManager::get());
    bool allow = false;
    
    do {
        if (mForeground.empty()) {
            allow = true;
        } else {
            allow = (mForeground.peek() == gCurrentProcess->pid);
        }

        if (false == allow) {
            pmm.yield();
        }
    } while(false == allow);

    // refuse to return anything except EOF when in EOF mode
    if (mEOF) {
        return TTY_EOF_MARKER;
    }

tryget:
    PS2Keyboard::key_event_t evt = tasks::keybqueue::readKey();
    if (evt.keycode == 0) return TTY_NO_INPUT; // out of input
    if (interceptChords(evt)) {
        if (mEOF) {
            // CTRL+D can be used as a chord to generate EOF, so
            // evaluate EOF again here
            return TTY_EOF_MARKER;
        } else {
            // chord didn't cause EOF - keep trying
            goto tryget;
        }
    }
    if (evt.down == false) {
        switch (evt.keycode) {
            case '\b':
            case '\n':
                return evt.keycode;
            default:
                if (evt.keycode >= ' ')
                    return evt.keycode;
        }
    }

    return TTY_NO_INPUT;
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
