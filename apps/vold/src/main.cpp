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

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include <kernel/syscalls/types.h>

#include "handlers.h"
#include "settings.h"
#include "automount.h"

static void notifyHandler(const char* kind, const char* id) {
    printf("[vold] new %s detected: '%s'\n", kind, id);
}

static volatile bool exitLoop = false;

static int eventLoop(FILE* f) {
    int numErrors = 0;
    while(!exitLoop) {
        message_t msg;
        bzero(&msg, sizeof(msg));
        while(0 == fread(&msg, sizeof(msg), 1, f));
        if (msg.header.payload_size != sizeof(diskmgr_msg_t)) {
            ++numErrors;
            printf("[vold] unknown message size: %u\n", msg.header.payload_size);
            continue;
        }

        diskmgr_msg_t *info = (diskmgr_msg_t*)msg.payload;
        switch (info->kind) {
            case diskmgr_msg_t::gNewController:
                onNewController( (const char*)info->payload );
                break;
            case diskmgr_msg_t::gNewDisk:
                onNewDisk( (const char*)info->payload );
                break;
            case diskmgr_msg_t::gNewVolume:
                onNewVolume( (const char*)info->payload );
                break;
            default:
                ++numErrors;
                printf("[vold] unknown message type: %u\n", info->kind);
                break;
        }
    }

    return numErrors;
}

int main() {
    vold_settings_t settings;
    if (!vold_settings_t::load(&settings)) {
        printf("[vold] unable to load settings\n");
        return 1;
    }

    if (settings.notify) {
        addControllerHandler([] (const char* id) -> void {
            notifyHandler("controller", id);
        });
        addDiskHandler([] (const char* id) -> void {
            notifyHandler("disk", id);
        });
        addVolumeHandler([] (const char* id) -> void {
            notifyHandler("volume", id);
        });
    }

    if (settings.automount)
        addVolumeHandler(automountVolumeHandler);

    FILE *f = fopen("/queues/diskmgr_events", "r");
    setvbuf(f, nullptr, _IOFBF, sizeof(message_t));

    return eventLoop(f);
}
