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

#include <parson/parson.h>
#include "settings.h"

#define VOLD_SETTINGS_PATH "/system/config/vold/settings.json"

#define LOAD_BOOL_SETTINGS(jsonName, fieldName) { \
    int __tmp = json_object_get_boolean(root_object, #jsonName); \
    if (__tmp == -1) return false; \
    dest-> fieldName = (__tmp == 1); \
}

bool vold_settings_t::load(vold_settings_t* dest) {
    JSON_Value *json_value = json_parse_file(VOLD_SETTINGS_PATH);
    if (json_value == nullptr) return false;
    JSON_Object *root_object = json_object(json_value);
    if (root_object == nullptr) return false;

    LOAD_BOOL_SETTINGS(automount, automount);
    LOAD_BOOL_SETTINGS(notify, notify);

    return true;
}
