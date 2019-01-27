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

#include "automount.h"
#include <stdlib.h>
#include <stdio.h>

#include <syscalls.h>
#include <kernel/syscalls/types.h>

#include <unordered_map>
#include <vector>

#include <parson/parson.h>

#define VOLD_SETTINGS_PATH "/system/config/vold/automount.json"

namespace {
    std::unordered_map<uint64_t, automount_rule_t> gRulesByUUID;
    std::vector<automount_rule_t> gRulesByName;
}

struct fdelete {
    fdelete(FILE* f) : fobj(f) {}
    operator FILE*() {
        return fobj;
    }
    ~fdelete() {
        if (fobj) fclose(fobj);
    }
private:
    FILE* fobj;
};

static bool processRule(int fid, automount_rule_t& rule, const char* volid) {
    int ok = 0;

    const auto& desired_mountpoint = rule.path;
    if (desired_mountpoint.size() > 0) {
        ok = mount_syscall(fid, desired_mountpoint.c_str());
        if (ok) {
            printf("[vold] error: volume '%s' cannot be mounted at '%s'\n",
                volid, desired_mountpoint.c_str());
            return false;
        }
    }

    ok = unmount_syscall(volid);
    if (ok) {
        printf("[vold] error: volume '%s' cannot be unmounted from default location\n",
            volid);
        return false;
    }

    return true;
}

void automountVolumeHandler(const char* volid) {
    std::string fpath;
    fpath.append_sprintf("/devices/disks/%s", volid);
    FILE* f = fopen(fpath.c_str(), "r");
    if (f == nullptr) {
        printf("[vold] error: volume '%s' does not match any device file\n", volid);
        return;
    }
    fdelete _fdel(f);

    int fid = fileno(f);
    int ok = mount_syscall(fid, volid);
    if (ok) {
        printf("[vold] error: volume '%s' cannot be mounted\n", volid);
        return;
    }

    filesystem_info_t fsinfo;
    if (0 != fsinfo_syscall(volid, &fsinfo)) {
        printf("[vold] error: volume '%s' cannot be configured\n", volid);
        return;
    }

    auto iter = gRulesByUUID.find(fsinfo.fs_uuid), end = gRulesByUUID.end();
    if (iter != end) {
        processRule(fid, iter->second, volid);
    } else {
        auto iter = gRulesByName.begin(), end = gRulesByName.end();
        while(iter != end) {
            if (volid == strstr(volid, iter->id.c_str())) {
                processRule(fid, *iter, volid);
                break;
            }
        }
    }
}

bool loadAutomountRules() {
    JSON_Value *json_value = json_parse_file(VOLD_SETTINGS_PATH);
    if (json_value == nullptr) return false;
    JSON_Array *root_array = json_array(json_value);
    if (root_array == nullptr) return false;

    const size_t count = json_array_get_count(root_array);
    for (size_t i = 0u; i < count; ++i) {
        JSON_Object *entry = json_array_get_object(root_array, i);
        if (entry == nullptr) return false;
        automount_rule_t rule;
        rule.uuid = (uint64_t)json_object_get_number(entry, "uuid");
        rule.path = json_object_get_string(entry, "mountpoint");
        const char* id = json_object_get_string(entry, "id");
        rule.id = id ? id : "";
        if (rule.uuid != 0)
            gRulesByUUID.emplace(rule.uuid, rule);
        if (rule.id != "")
            gRulesByName.push_back(rule);
    }

    return true;
}
