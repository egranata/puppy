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

#include <kernel/tty/file.h>
#include <kernel/process/manager.h>
#include <kernel/log/log.h>
#include <kernel/syscalls/types.h>
#include <kernel/panic/panic.h>

LOG_TAG(TTYFILE, 2);
LOG_TAG(TTYEOF, 2);

TTYFile::TTYFile(TTY* tty) : mTTY(tty) {
    mInput.clear();
}

void TTYFile::setTTY(TTY* tty) {
    if (mTTY != nullptr) PANIC("cannot change underlying TTY");
    mTTY = tty;
}

bool TTYFile::input_t::appendOne(char c) {
    if (mNextWrite == sizeof(buf)) return false;
    buf[mNextWrite++] = c;
    return true;
}
bool TTYFile::input_t::undoAppend(char* c) {
    if (emptyWrite()) return false;
    --mNextWrite;
    if (c != nullptr) *c = buf[mNextWrite];
    return true;
}
bool TTYFile::input_t::consumeOne(char* c) {
    if (emptyRead()) return false;
    if(c) {
        *c = buf[mNextRead++];
    } else {
        mNextRead++;
    }
    return true;
}
int TTYFile::input_t::peekOne() {
    if (emptyRead()) return -1;
    return buf[mNextRead];
}
bool TTYFile::input_t::emptyWrite() const {
    return mNextWrite == 0;
}
bool TTYFile::input_t::emptyRead() const {
    return mNextWrite == mNextRead;
}
void TTYFile::input_t::clear() {
    bzero(this, sizeof(*this));
}

bool TTYFile::seek(size_t) {
    return false;
}

// TODO: this should be using waitqueues - not yielding
char TTYFile::procureOne() {
ch:
    auto c = mTTY->read();
    if (c == TTY::TTY_NO_INPUT) {
        ProcessManager::get().yield();
        goto ch;
    }
    if (c == TTY::TTY_EOF_MARKER) {
        return 0xFF;
    }
    return (char)c;
}

size_t TTYFile::read(size_t n, char* b) {
entry:
    TAG_DEBUG(TTYFILE, "trying to consume up to %u bytes from the TTY - mode is %x", n, mMode);
    if (n == 0) return n;

    while (mMode == mode_t::READ_FROM_IRQ) {
        bool swallow = false;
        auto c = procureOne();
        TAG_DEBUG(TTYFILE, "got a byte: %x", c);
        switch (c) {
            case '\b':
                if (mInput.emptyWrite()) {
                    swallow = true;
                } else {
                    mInput.undoAppend(nullptr);
                }
                TAG_DEBUG(TTYFILE, "processed a backspace");
                break;
            case '\n':
                mInput.appendOne(c);
                mMode = mode_t::CONSUME_BUFFER;
                TAG_DEBUG(TTYFILE, "processed newline");
                break;
            case input_t::EOF_MARKER:
                mInput.appendOne(c);
                mMode = mode_t::CONSUME_BUFFER;
                TAG_DEBUG(TTYEOF, "processed EOF");
                break;
            default:
                if (false == mInput.appendOne(c)) {
                    // this buffer is full - let it be consumed
                    mMode = mode_t::CONSUME_BUFFER;
                }
                TAG_DEBUG(TTYFILE, "processed a character");
                break;
        }
        TAG_DEBUG(TTYFILE, "swallow = %s", swallow ? "yes" : "no");
        if (!swallow) {
            write(1, &c);
        }
    }

    if (mMode == mode_t::ONLY_EOF) {
        return 0;
    }

    auto n0 = 0u;
    while(mMode == mode_t::CONSUME_BUFFER) {
        if (mInput.emptyRead()) {
            if (n0 == 0) {
                TAG_DEBUG(TTYFILE, "the TTY is empty and nothing was read, but we're not in EOF mode; try again");
                mMode = mode_t::READ_FROM_IRQ;
                goto entry;
            } else {
                TAG_DEBUG(TTYFILE, "the TTY is empty - get out with %u bytes read", n0);
                mInput.clear();
                mMode = mode_t::READ_FROM_IRQ;
                break;
            }
        } else {
            int peek = mInput.peekOne();
            if (peek == input_t::EOF_MARKER) {
                if (n0 == 0) {
                    TAG_DEBUG(TTYEOF, "the TTY received EOF and nothing was read; consume it and get out");
                    mInput.consumeOne(nullptr);
                    mMode = mode_t::ONLY_EOF;
                    return 0;
                } else {
                    TAG_DEBUG(TTYEOF, "the TTY received EOF but %u bytes were read - get out now and consume later", n0);
                    break;
                }
            } else {
                mInput.consumeOne(b);
                TAG_DEBUG(TTYFILE, "consumed a byte: %x", *b);
                ++b;
            }
        }
        if (++n0 == n) break;
    }

    return n0;
}

size_t TTYFile::write(size_t s, char* buffer) {
    mTTY->write(s, buffer);
    return s;
}

bool TTYFile::stat(stat_t&) {
    return false;
}

uintptr_t TTYFile::ioctl(uintptr_t a1, uintptr_t a2) {
    LOG_DEBUG("ttyfile %p ioctl(%u,%u)", this, a1, a2);
    switch (a1) {
        case IOCTL_FOREGROUND:
            // TODO: should processes be allowed to push themselves to the foreground freely?
            mTTY->pushfg((kpid_t)a2);
            return 1;
        case IOCTL_BACKGROUND:
            mTTY->popfg();
            return 1;
        case IOCTL_MOVECURSOR: {
            uint16_t row = a2 & 0xFFFF;
            uint16_t col = a2 >> 16;
            mTTY->setPosition(row, col);
            return 1;
        }
        case IOCTL_VISIBLE_AREA: {
            uint16_t *pi = (uint16_t*)a2;
            mTTY->getSize(pi, pi+1);
            return 1;
        }
        case IOCTL_CURSOR_POS: {
            uint16_t *pi = (uint16_t*)a2;
            mTTY->getPosition(pi, pi+1);
            return 1;
        }
        case IOCTL_GET_FG_COLOR: {
            uint32_t *clr = (uint32_t*)a2;
            mTTY->getForegroundColor(clr);
            return 1;
        }
        case IOCTL_GET_BG_COLOR: {
            uint32_t *clr = (uint32_t*)a2;
            mTTY->getBackgroundColor(clr);
            return 1;
        }
        case IOCTL_SET_FG_COLOR: {
            mTTY->setForegroundColor(a2);
            return 1;
        }
        case IOCTL_SET_BG_COLOR: {
            mTTY->setBackgroundColor(a2);
            return 1;
        }
        case TIOCGWINSZ: {
            winsize_t* ws = (winsize_t*)a2;
            uint16_t r=0,c=0;
            mTTY->getSize(&r, &c);
            ws->ws_col = c;
            ws->ws_row = r;
            // TODO: do not hardcode font height and width
            ws->ws_xpixel = c * 8;
            ws->ws_ypixel = c * 16;
            return 1;
        }
        default:
            return 0;
    }
}

