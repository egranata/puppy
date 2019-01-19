/*
 * Copyright 2019 Google LLC
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

#include <kernel/drivers/ioports/driver.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/i386/ioports.h>
#include <kernel/process/current.h>
#include <kernel/fs/vfs.h>
#include <kernel/drivers/serial/serial.h>

namespace boot::ioports {
    uint32_t init() {
        Serial::get().reservePorts();

        IOPortsDevice::get();
        return 0;
    }
}

IOPortsDevice& IOPortsDevice::get() {
    static IOPortsDevice gDevice;

    return gDevice;
}

MemFS::Directory* IOPortsDevice::deviceDirectory() {
    return mDeviceDirectory;
}

namespace {
    class IOPortFile : public Filesystem::File {
        public:
            IOPortFile(IOPortsManager::ioport_t port) {
                mPort = IOPortsManager::get().getPort(port);
            }
            ~IOPortFile() {
                IOPortsManager::get().freePort(mPort->port());
                delete mPort;
                mPort = nullptr;
            }

            bool doStat(stat_t&) override {
                return true;
            }
            bool seek(size_t) override {
                return false;
            }
            bool tell(size_t*) override {
                return false;
            }
            size_t read(size_t n, char* buf) override {
                if (mPort == nullptr) return 0;
                switch (n) {
                    case 1: {
                        *buf = mPort->read8();
                        return n;
                    }
                    case 2: {
                        *((uint16_t*)buf) = mPort->read16();
                        return n;
                    }
                    case 4: {
                        *((uint32_t*)buf) = mPort->read32();
                        return n;
                    }
                    default:
                        return 0;
                }
            }
            size_t write(size_t n, char* buf) override {
                if (mPort == nullptr) return 0;
                switch (n) {
                    case 1: {
                        mPort->write8(*(uint8_t*)buf);
                        return n;
                    }
                    case 2: {
                        mPort->write16(*(uint16_t*)buf);
                        return n;
                    }
                    case 4: {
                        mPort->write32(*(uint32_t*)buf);
                        return n;
                    }
                    default:
                        return 0;
                }
            }
        private:
            IOPortsManager::IOPort* mPort;
    };

    class IOPortFilesystem : public Filesystem {
        public:
            IOPortFilesystem* newObject() {
                openObject();
                return this;
            }
            File* doOpen(const char*, uint32_t) override {
                return nullptr;
            }
            Directory* doOpendir(const char*) override {
                return nullptr;
            }
            void doClose(FilesystemObject* object) override {
                delete object;
            }
    };

    class DeviceFile : public MemFS::File {
        private:
            IOPortFilesystem mFilesystem;
        public:
            DeviceFile() : MemFS::File("ioports") {
                kind(file_kind_t::chardevice);
            }

            uintptr_t ioctl(uintptr_t a, uintptr_t b) override {
                switch (a) {
                    case IOPortsDevice::X86_IOPORT_OPEN: {
                        IOPortsManager::ioport_t portnum = (IOPortsManager::ioport_t)b;
                        bool ok = IOPortsManager::get().allocatePort(portnum);
                        if (!ok) return -1;
                        Filesystem::File *file = new IOPortFile(portnum);
                        VFS::filehandle_t fh;
                        fh.filesystem = mFilesystem.newObject();
                        fh.object = file;
                        size_t descriptor = 0;
                        if (false == gCurrentProcess->fds.set(fh, descriptor)) return -1;
                        return descriptor;
                    }
                    default:
                        return this->File::ioctl(a,b);
                }
            }

            delete_ptr<MemFS::FileBuffer> content() override {
                return new MemFS::EmptyBuffer();
            }
    };
}

IOPortsDevice::IOPortsDevice() {
    DevFS& devfs(DevFS::get());
    mDeviceDirectory = devfs.getRootDirectory();
    mDeviceDirectory->add(new DeviceFile());
}
