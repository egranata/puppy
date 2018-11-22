/*
 * Copyright 2018 Google LLC
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

#include <kernel/sys/config.h>
#include <kernel/boot/phase.h>
#include <kernel/boot/bootinfo.h>
#include <kernel/libc/string.h>
#include <muzzle/stdlib.h>
#include <kernel/log/log.h>

kernel_config_t* gKernelConfiguration() {
    static kernel_config_t gConfig;

    return &gConfig;
}

kernel_config_t::kernel_config_t() {
    logging.value = config_logging::gFullLogging;
    mainfs.value = nullptr;
    logsize.value = 64;
}

namespace {
    template<typename X>
    static X min(X a, X b) {
        return a < b ? a : b;
    }

    static bool matches(const char* x, const char* y) {
        auto lenx = strlen(x);
        auto leny = strlen(y);
        if (lenx == leny) {
            return 0 == strncmp(x, y, lenx);
        } else {
            return false;
        }
    }
}

static void parseOption(kernel_config_t* kcfg, char* option) {
    char* eq = strchr(option, '=');
    char* key = option;
    char* value = eq + 1;
    *eq = 0;

    LOG_DEBUG("found kernel option \"%s\" = \"%s\"", key, value);

    if (matches(key, "loglevel")) {
        kcfg->logging.value = atoi(value);
    }
    else if (matches(key, "mainfs")) {
        kcfg->mainfs.value = strdup(value);
    } else if (matches(key, "logsize")) {
        kcfg->logsize.value = atoi(value);
        if (kcfg->logsize.value == 0) kcfg->logsize.value = 64;
    }
}

static kernel_config_t* parseKernelCommandLine() {
    auto kcfg = gKernelConfiguration();

    auto&& kcmdline = bootcmdline()->cmdline;

    auto optinit = &kcmdline[0];

    bool first = true;
    const char* delim = " ";
    char* saveptr = 0;

    while(true) {
        char* next = strtok_r(first ? optinit : nullptr, delim, &saveptr);
        first = false;
        if(next == nullptr) break;
        parseOption(kcfg, next);
    }

    return kcfg;
}

namespace boot::config {
    uint32_t init() {
        parseKernelCommandLine();

        return bootphase_t::gSuccess;
    }
}

