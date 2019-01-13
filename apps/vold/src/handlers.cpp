// Copyright 2019 Google LLC
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

#include <list>
#include <algorithm>

#include "handlers.h"

static std::list<new_controller_handler_t> gControllerHandlers;
static std::list<new_disk_handler_t> gDiskHandlers;
static std::list<new_volume_handler_t> gVolumeHandlers;

void addControllerHandler(new_controller_handler_t handler) {
    gControllerHandlers.push_back(handler);
}
void addDiskHandler(new_disk_handler_t handler) {
    gDiskHandlers.push_back(handler);
}
void addVolumeHandler(new_volume_handler_t handler) {
    gVolumeHandlers.push_back(handler);
}

void onNewController(const char* id) {
    std::for_each(gControllerHandlers.begin(), gControllerHandlers.end(), [id] (const auto& f) -> void {
        f(id);
    });
}
void onNewDisk(const char* id) {
    std::for_each(gDiskHandlers.begin(), gDiskHandlers.end(), [id] (const auto& f) -> void {
        f(id);
    });
}
void onNewVolume(const char* id) {
    std::for_each(gVolumeHandlers.begin(), gVolumeHandlers.end(), [id] (const auto& f) -> void {
        f(id);
    });
}
