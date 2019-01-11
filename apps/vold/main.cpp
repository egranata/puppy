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

int main() {
    FILE *f = fopen("/queues/diskmgr_events", "r");
    setvbuf(f, nullptr, _IOFBF, sizeof(message_t));
    while(true) {
        message_t msg;
        bzero(&msg, sizeof(msg));
        while(0 == fread(&msg, sizeof(msg), 1, f));
        if (msg.header.payload_size != sizeof(diskmgr_msg_t)) {
            printf("[vold] unknown message size: %u\n", msg.header.payload_size);
            continue;
        }

        diskmgr_msg_t *info = (diskmgr_msg_t*)msg.payload;
        switch (info->kind) {
            case diskmgr_msg_t::gNewController:
                printf("[vold] new controller detected\n");
                break;
            case diskmgr_msg_t::gNewDisk:
                printf("[vold] new disk detected '%s'\n", info->payload);
                break;
            case diskmgr_msg_t::gNewVolume:
                printf("[vold] new volume detected '%s'\n", info->payload);
                break;
            default:
                printf("[vold] unknown message type: %u\n", info->kind);
                break;
        }
    }
}
