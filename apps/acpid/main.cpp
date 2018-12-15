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
#include <unistd.h>

#include <parson/parson.h>

#include <syscalls.h>
#include <sys/collect.h>
#include <sys/ioctl.h>
#include <sys/process.h>

#include <libshell/system.h>

#include <optional>
#include <string>
#include <vector>

struct config_data {
    std::string event_file_path;
    FILE *event_file;
    int event_fd;

    int num_events;
    int window_ms;

    std::vector<int> event_times;

    std::string action_command;
};

static std::string readFile(const char* path) {
    FILE* fd = fopen(path, "r");
    if (fd == nullptr) return nullptr;

    std::string str;
    while(true) {
        int c = fgetc(fd);
        if (c == EOF) break;
        str.append_sprintf("%c", c);
    }

    fclose(fd);

    return str;
}

static std::optional<std::string> readJSONString(JSON_Object *root, const char* key) {
    auto data = json_object_get_string(root, key);
    if (data) return std::optional<std::string>(std::string(data));
    return std::nullopt;
}

static std::optional<int> readJSONInteger(JSON_Object *root, const char* key) {
    auto data = (int)(json_object_get_number(root, key));
    if (data) return std::optional<int>(data);
    return std::nullopt;
}

#define VALID_OR_FAIL(x) if (!x) { return false; } else

static bool loadConfigData(const char* path, config_data *cfg_data) {
    if (path == nullptr || 0 == *path) return false;
    auto string_data = readFile(path);
    if (string_data.empty()) return false;

    JSON_Value *json_value = json_parse_string(string_data.c_str());
    if (json_value == nullptr) return false;
    JSON_Object *root_object = json_object(json_value);
    if (root_object == nullptr) return false;

    auto event_file = readJSONString(root_object, "event_file");
    VALID_OR_FAIL(event_file) {
        cfg_data->event_file_path = *event_file;
        cfg_data->event_file = fopen(cfg_data->event_file_path.c_str(), "r");
        VALID_OR_FAIL(cfg_data->event_file) {
            cfg_data->event_fd = fileno(cfg_data->event_file);
        }
    };

    auto num_events = readJSONInteger(root_object, "num_events");
    VALID_OR_FAIL(num_events) {
        cfg_data->num_events = *num_events;
    }

    auto window_ms = readJSONInteger(root_object, "window_ms");
    VALID_OR_FAIL(window_ms) {
        cfg_data->window_ms = *window_ms;
    }

    auto action_command = readJSONString(root_object, "action_command");
    VALID_OR_FAIL(action_command) {
        cfg_data->action_command = *action_command;
    }

    return true;
}

#undef VALID_OR_FAIL

static int uptime() {
    auto uptime_str = readFile("/devices/time/uptime");
    return atoi(uptime_str.c_str());
}

static bool pushNewEvent(config_data& cfg_data) {
    auto now = uptime();
    if (cfg_data.event_times.size() == cfg_data.num_events) {
        cfg_data.event_times.pop_back();
    }
    cfg_data.event_times.insert(cfg_data.event_times.begin(), now);

    if (cfg_data.event_times.size() == cfg_data.num_events) {
        auto newest = cfg_data.event_times.front();
        auto oldest = cfg_data.event_times.back();
        if ((newest-oldest) <= cfg_data.window_ms) {
            printf("[acpid] running '%s' in response to events on '%s'\n",
                cfg_data.action_command.c_str(),
                cfg_data.event_file_path.c_str());
            libShellSupport::system(cfg_data.action_command.c_str());
            return true;
        }
    }

    return false;
}

static int loop(config_data& cfg_data) {
    while (true) {
        wait1_syscall(cfg_data.event_fd, 0);
        pushNewEvent(cfg_data);
    }
}

int main(int argc, char** argv) {
    if (argc == 1) return 1;
    config_data cfg_data;
    if (!loadConfigData(argv[1], &cfg_data)) return 1;
    return loop(cfg_data);
}
