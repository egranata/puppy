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
#include <kernel/drivers/ps2/keyboard.h>
#include <kernel/process/current.h>

LOG_TAG(RAWTTY, 0);
LOG_TAG(TTYFILE, 2);
LOG_TAG(TTYEOF, 2);

TTYFile::TTYFile(TTY* tty) : mTTY(tty) {
    kind(file_kind_t::tty);
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

bool TTYFile::tell(size_t*) {
    return false;
}

// TODO: this should be using waitqueues - not yielding
key_event_t TTYFile::procureOne() {
ch:
    auto key_evt = mTTY->readKeyEvent();
    if (key_evt.keycode == 0) {
        ProcessManager::get().yield();
        goto ch;
    } else {
        return key_evt;
    }
}

#define CTRL evt.ctrldown
#define ALT evt.altdown
#define KEY(x) (evt.keycode == x)

bool TTYFile::interceptChords_Canonical(const key_event_t& evt, bool* eofHappened) {
    if (evt.down) return false;
    if (CTRL && ALT && (KEY('K') || KEY('k'))) {
        mTTY->clearScreen();
        return true;
    }

    if (CTRL && (KEY('C') || KEY('c'))) {
        mTTY->killForegroundProcess();
        return true;
    }

    if (CTRL && (KEY('D') || KEY('d'))) {
        *eofHappened = true;
        return true;
    }
    return false;
}

void TTYFile::processOne_Canonical(key_event_t evt) {
    bool eof = false;
    bool swallow = false;

    if (interceptChords_Canonical(evt, &eof)) {
        if (eof) {
            mInput.appendOne(input_t::EOF_MARKER);
            mMode = mode_t::CONSUME_BUFFER;
        }
        return;
    }

    uint16_t ch = evt.keycode;
    TAG_DEBUG(TTYFILE, "got a character: 0x%x", ch);
    // ignore HBS characters; they are not for us
    if (ch & 0xFF00) {
        TAG_DEBUG(TTYFILE, "HBS characters ignored by TTYFile");
        return;
    }
    char c = (char)(ch & 0x00FF);
    TAG_DEBUG(TTYFILE, "got a byte: 0x%x", c);
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

#define RAW_COMBO(...) { \
    char cs[] = {__VA_ARGS__}; \
    size_t n = sizeof(cs) / sizeof(cs[0]); \
    for (size_t i = 0; i < n; ++i) { \
        mInput.appendOne(cs[i]); \
    } \
}
#define ESCAPE 27

bool TTYFile::interceptChords_Raw(const key_event_t& evt, bool*) {
    if (evt.down) return false;
    if (CTRL) {
        switch (evt.keycode) {
            case 'A':
            case 'a':
                mInput.appendOne(1);
                return true;
            case 'C':
            case 'c':
                mInput.appendOne(3);
                return true;
            case 'D':
            case 'd':
                mInput.appendOne(4);
                return true;
            case 'E':
            case 'e':
                mInput.appendOne(5);
                return true;
            case 'K':
            case 'k':
                mInput.appendOne(11);
                return true;
            case 'U':
            case 'u':
                mInput.appendOne(21);
                return true;
            case 'W':
            case 'w':
                mInput.appendOne(23);
                return true;
        }
    }
    return false;
}

void TTYFile::processOne_Raw(key_event_t evt) {
    bool ignored = false;
    bool eof = false;

    if (interceptChords_Raw(evt, &eof)) {
        if (eof) {
            mInput.appendOne(input_t::EOF_MARKER);
        }
        mMode = mode_t::CONSUME_BUFFER;
        return;
    }

    uint16_t ch = evt.keycode;
    TAG_DEBUG(RAWTTY, "got a character: 0x%x", ch);
    if (ch & 0xFF00) {
        if      (ch == key_event_t::UP)  RAW_COMBO(ESCAPE, '[', 'A')
        else if (ch == key_event_t::DWN) RAW_COMBO(ESCAPE, '[', 'B')
        else if (ch == key_event_t::RHT) RAW_COMBO(ESCAPE, '[', 'C')
        else if (ch == key_event_t::LFT) RAW_COMBO(ESCAPE, '[', 'D')
        else if (ch == key_event_t::F1)  RAW_COMBO(ESCAPE, 'O', 'P')
        else if (ch == key_event_t::F2)  RAW_COMBO(ESCAPE, 'O', 'Q')
        else if (ch == key_event_t::F3)  RAW_COMBO(ESCAPE, 'O', 'R')
        else if (ch == key_event_t::F4)  RAW_COMBO(ESCAPE, 'O', 'S')
        else if (ch == key_event_t::F5)  RAW_COMBO(ESCAPE, 'O', 't')
        else if (ch == key_event_t::F6)  RAW_COMBO(ESCAPE, 'O', 'u')
        else if (ch == key_event_t::F7)  RAW_COMBO(ESCAPE, 'O', 'v')
        else if (ch == key_event_t::F8)  RAW_COMBO(ESCAPE, 'O', '1')
        else if (ch == key_event_t::F9)  RAW_COMBO(ESCAPE, 'O', 'w')
        else if (ch == key_event_t::F10) RAW_COMBO(ESCAPE, 'O', 'x')
        else if (ch == key_event_t::DEL) RAW_COMBO(ESCAPE, '[', '3', '~')
        else {
            ignored = true;
            TAG_DEBUG(RAWTTY, "unknown HBS sequence - ignoring");
        }
    }
    else {
        char c = (char)(ch & 0x00FF);
        mInput.appendOne(c);
    }
    if (!ignored) mMode = mode_t::CONSUME_BUFFER;
}

#undef RAW_COMBO

size_t TTYFile::read(size_t n, char* b) {
entry:
    TAG_DEBUG(TTYFILE, "trying to consume up to %u bytes from the TTY - mode is 0x%x", n, mMode);
    if (n == 0) return n;

    while (mMode == mode_t::READ_FROM_IRQ) {
        auto ch = procureOne();
        switch (mDiscipline) {
            case discipline_t::CANONICAL:
                processOne_Canonical(ch);
                break;
            case discipline_t::RAW:
                processOne_Raw(ch);
                break;
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
                TAG_DEBUG(TTYFILE, "consumed a byte: 0x%x", *b);
                ++b;
            }
        }
        if (++n0 == n) break;
    }

    return n0;
}

#define CURRENT_CSI_INPUT mEscapeSequenceInput[mCurrentEscapeSequenceInput]
#define NUM_CSI_INPUTS (mCurrentEscapeSequenceInput+1)

size_t TTYFile::write(size_t s, char* buffer) {
    const size_t s0 = s;

    for(; s; ++buffer, --s) {
        char c = *buffer;
        bool writeThis = true;
        switch (c) {
            case ESCAPE: {
                writeThis = false;
                mEscapeStatus = escape_sequence_status_t::ESC;
                bzero(mEscapeSequenceInput, sizeof(mEscapeSequenceInput));
                mCurrentEscapeSequenceInput = 0;
            } break;
            case '[': {
                if (mEscapeStatus == escape_sequence_status_t::ESC) {
                    writeThis = false;
                    mEscapeStatus = escape_sequence_status_t::CSI;
                }
            } break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    CURRENT_CSI_INPUT = 10 * CURRENT_CSI_INPUT + (c - '0');
                    TAG_DEBUG(RAWTTY, "c = %c, CURRENT_CSI_INPUT = %d", c, CURRENT_CSI_INPUT);
                }
            } break;
            case ';': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    if (mCurrentEscapeSequenceInput == gNumEscapeInputs - 1) {
                        LOG_WARNING("escape sequence using too many input entries; ignoring");
                    } else ++mCurrentEscapeSequenceInput;
                } break;
            }
            case 'A': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    mEscapeStatus = escape_sequence_status_t::OFF;
                    if (CURRENT_CSI_INPUT == 0) CURRENT_CSI_INPUT = 1;
                    TAG_DEBUG(RAWTTY, "cursor move up by %d", CURRENT_CSI_INPUT);
                    uint16_t row = 0, col = 0;
                    mTTY->getPosition(&row, &col);
                    if (row >= CURRENT_CSI_INPUT) row -= CURRENT_CSI_INPUT;
                    else row = 0;
                    mTTY->setPosition(row, col);
                }
            } break;
            case 'B': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    mEscapeStatus = escape_sequence_status_t::OFF;
                    if (CURRENT_CSI_INPUT == 0) CURRENT_CSI_INPUT = 1;
                    TAG_DEBUG(RAWTTY, "cursor move down by %d", CURRENT_CSI_INPUT);
                    uint16_t row = 0, col = 0;
                    mTTY->getPosition(&row, &col);
                    row += CURRENT_CSI_INPUT;
                    mTTY->setPosition(row, col);
                }
            } break;
            case 'C': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    mEscapeStatus = escape_sequence_status_t::OFF;
                    if (CURRENT_CSI_INPUT == 0) CURRENT_CSI_INPUT = 1;
                    TAG_DEBUG(RAWTTY, "cursor move forward by %d", CURRENT_CSI_INPUT);
                    uint16_t row = 0, col = 0;
                    mTTY->getPosition(&row, &col);
                    col += CURRENT_CSI_INPUT;
                    mTTY->setPosition(row, col);
                }
            } break;
            case 'D': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    mEscapeStatus = escape_sequence_status_t::OFF;
                    if (CURRENT_CSI_INPUT == 0) CURRENT_CSI_INPUT = 1;
                    TAG_DEBUG(RAWTTY, "cursor move backwards by %d", CURRENT_CSI_INPUT);
                    uint16_t row = 0, col = 0;
                    mTTY->getPosition(&row, &col);
                    if (col >= CURRENT_CSI_INPUT) col -= CURRENT_CSI_INPUT;
                    else col = 0;
                    mTTY->setPosition(row, col);
                }
            } break;
            case 'H': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    mEscapeStatus = escape_sequence_status_t::OFF;
                    TAG_DEBUG(RAWTTY, "cursor move to origin", CURRENT_CSI_INPUT);
                    mTTY->setPosition(0, 0);
                }
            } break;
            case 'J': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    mEscapeStatus = escape_sequence_status_t::OFF;
                    if (CURRENT_CSI_INPUT == 2) {
                        TAG_DEBUG(RAWTTY, "screen clear command");
                        mTTY->clearScreen();
                    } else {
                        TAG_DEBUG(RAWTTY, "unknown screen clear command %d", CURRENT_CSI_INPUT);
                    }
                }
            } break;
            case 'K': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    mEscapeStatus = escape_sequence_status_t::OFF;
                    TAG_DEBUG(RAWTTY, "line clear command %d", CURRENT_CSI_INPUT);
                    if (CURRENT_CSI_INPUT == 0) mTTY->clearLine(false, true);
                    if (CURRENT_CSI_INPUT == 1) mTTY->clearLine(true, false);
                    if (CURRENT_CSI_INPUT == 2) mTTY->clearLine(true, true);
                }
            } break;
            case 'm': {
                if (mEscapeStatus == escape_sequence_status_t::CSI) {
                    writeThis = false;
                    mEscapeStatus = escape_sequence_status_t::OFF;
                    if (NUM_CSI_INPUTS == 1) {
                        if (CURRENT_CSI_INPUT == 0) mTTY->resetGraphics();
                        if (CURRENT_CSI_INPUT == 7) mTTY->swapColors();
                        if (CURRENT_CSI_INPUT >= 30 && CURRENT_CSI_INPUT < 39) mTTY->setANSIForegroundColor(CURRENT_CSI_INPUT);
                        if (CURRENT_CSI_INPUT >= 40 && CURRENT_CSI_INPUT < 49) mTTY->setANSIBackgroundColor(CURRENT_CSI_INPUT);
                    }
                    if (NUM_CSI_INPUTS == 5) {
                        if (mEscapeSequenceInput[0] == 38 && mEscapeSequenceInput[1] == 2) {
                            mTTY->setANSIForegroundColor(mEscapeSequenceInput[2], mEscapeSequenceInput[3], mEscapeSequenceInput[4]);
                        }
                        if (mEscapeSequenceInput[0] == 48 && mEscapeSequenceInput[1] == 2) {
                            mTTY->setANSIBackgroundColor(mEscapeSequenceInput[2], mEscapeSequenceInput[3], mEscapeSequenceInput[4]);
                        }
                    }
                }
            } break;
        }
        if (writeThis) {
            mTTY->write(1, &c);
        }
    }

    return s0;
}

#undef NUM_CSI_INPUTS
#undef CURRENT_CSI_INPUT


bool TTYFile::doStat(stat_t& stat) {
    stat.kind = file_kind_t::tty;
    stat.size = 0;
    stat.time = 0;
    return true;
}

uintptr_t TTYFile::ioctl(uintptr_t a1, uintptr_t a2) {
    LOG_DEBUG("ttyfile 0x%p ioctl(%u,%u)", this, a1, a2);
    switch (a1) {
        case IOCTL_FOREGROUND:
            // TODO: should processes be allowed to push themselves to the foreground freely?
            mTTY->pushfg((kpid_t)a2);
            return 1;
        case IOCTL_BACKGROUND:
            mTTY->popfg(gCurrentProcess->pid);
            return 1;
        case IOCTL_MOVECURSOR: {
            uint16_t col = a2 & 0xFFFF;
            uint16_t row = a2 >> 16;
            mTTY->setPosition(row, col);
            return 1;
        }
        case IOCTL_VISIBLE_AREA: {
            uint16_t *pi = (uint16_t*)a2;
            mTTY->getSize(pi+1, pi);
            return 1;
        }
        case IOCTL_CURSOR_POS: {
            uint16_t *pi = (uint16_t*)a2;
            mTTY->getPosition(pi+1, pi);
            return 1;
        }
        case IOCTL_GET_FG_COLOR: {
            uint32_t *clr = (uint32_t*)a2;
            mTTY->getConfiguredForegroundColor(clr);
            return 1;
        }
        case IOCTL_GET_BG_COLOR: {
            uint32_t *clr = (uint32_t*)a2;
            mTTY->getConfiguredBackgroundColor(clr);
            return 1;
        }
        case IOCTL_SET_FG_COLOR: {
            mTTY->setConfiguredForegroundColor(a2);
            return 1;
        }
        case IOCTL_SET_BG_COLOR: {
            mTTY->setConfiguredBackgroundColor(a2);
            return 1;
        }
        case IOCTL_DISCIPLINE_RAW : {
            mDiscipline = discipline_t::RAW;
            return 1;
        }
        case IOCTL_DISCIPLINE_CANONICAL: {
            mDiscipline = discipline_t::CANONICAL;
            return 1;
        }
        case IOCTL_DISCIPLINE_GET: {
            int *discipline = (int*)a2;
            *discipline = (int)mDiscipline;
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

#undef ESCAPE
