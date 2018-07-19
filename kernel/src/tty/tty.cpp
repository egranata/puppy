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

TTY::TTY() : mWriteSemaphore("tty", 1, 1), mForeground(), mOutQueue(), mInQueue(), mFramebuffer(Framebuffer::get()) {}

void TTY::write(const char* buffer) {
    mWriteSemaphore.wait();

    mFramebuffer.write(buffer);

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

bool TTY::interceptChords(const PS2Keyboard::key_event_t& evt) {
    if (evt.down) return false;
    if (evt.ctrldown && evt.altdown && evt.keycode == 'k') {
        mFramebuffer.cls();
        return true;
    }
    return false;
}

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

    // TODO: make a global input queue
    auto d1 = PS2Controller::get().getDevice1();
    if (d1 && d1->getType() == PS2Controller::Device::Type::KEYBOARD) {
        PS2Keyboard::key_event_t evt;
        while (true) {
            evt = ((PS2Keyboard*)d1)->next();
            if (evt.keycode == 0) return -1; // out of input
            if (interceptChords(evt)) {
                continue;
            } else {
                break;
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
    }

    return -1;
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
    mFramebuffer.setbg(Framebuffer::color_t(color));
}
