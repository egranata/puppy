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

#include <kernel/synch/eventfs.h>
#include <kernel/boot/phase.h>
#include <kernel/fs/vfs.h>
#include <kernel/log/log.h>
#include <kernel/syscalls/types.h>

namespace boot::eventfs {
    uint32_t init() {
        bool ok = VFS::get().mount("events", EventFS::get());
        return ok ? 0 : 0xFF;
    }
    bool fail(uint32_t) {
        bootphase_t::printf("unable to mount /events");
        return bootphase_t::gPanic;
    }
}

EventFS::Store::Store() : KeyedStore() {}

Event* EventFS::Store::getOrCreate(const char* name) {
    Event *event = getOrNull(name);
    if (event == nullptr) event = makeOrNull(name);
    return event;
}

bool EventFS::Store::release(const char* key) {
    return this->KeyedStore::release(key);
}

EventFS::EventFS() : mEvents() {}

class EventFile : public Filesystem::File {
public:
    EventFile(Event* sema) : mEvent(sema) {
        kind(file_kind_t::event);
    }

    bool doStat(stat_t& stat) override {
        stat.kind = file_kind_t::event;
        stat.size = 0;
        stat.time = 0;
        return true;
    }

    bool seek(size_t) override {
        return false;
    }
    bool tell(size_t*) override {
        return false;
    }
    size_t read(size_t, char*) override {
        return 0;
    }
    size_t write(size_t, char*) override {
        return 0;
    }

    WaitableObject* waitable() override {
        return mEvent;
    }

    uintptr_t ioctl(uintptr_t a, uintptr_t b) override {
        switch (a) {
            case event_ioctl_t::IOCTL_EVENT_RAISE:
                mEvent->raise(b == 0);
                return 1;
            case event_ioctl_t::IOCTL_EVENT_LOWER:
                mEvent->lower();
                return 1;
        }
        return 0;
    }

    Event* event() { return mEvent; }
private:
    Event *mEvent;
};

Filesystem::File* EventFS::open(const char* name, uint32_t) {
    Event* event = mEvents.getOrCreate(name);
    if (event == nullptr) return nullptr;
    return new EventFile(event);
}

void EventFS::doClose(FilesystemObject* file) {
    if (file == nullptr) return;

    if (file->kind() != file_kind_t::event) {
        PANIC("EventFS cannot close anything but events");
    }

    EventFile *sFile = (EventFile*)file;
    Event* sema = sFile->event();
    const bool erased = mEvents.release(sema->key());

    LOG_INFO("event file 0x%p is being deleted; event 0x%p was%serased",
        sFile, sema, erased ? " " : " not ");

    delete sFile;
}

EventFS* EventFS::get() {
    static EventFS gFS;

    return &gFS;
}

