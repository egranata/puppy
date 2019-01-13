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

#ifndef VOLD_HANDLERS
#define VOLD_HANDLERS

#include <functional>

typedef std::function<void(const char*)> new_controller_handler_t;
typedef std::function<void(const char*)> new_disk_handler_t;
typedef std::function<void(const char*)> new_volume_handler_t;

void addControllerHandler(new_controller_handler_t);
void addDiskHandler(new_disk_handler_t);
void addVolumeHandler(new_volume_handler_t);

void onNewController(const char* id);
void onNewDisk(const char* id);
void onNewVolume(const char* id);

#endif
