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

#include <kernel/log/log.h>
#include <kernel/libc/sprint.h>
#include <kernel/drivers/serial/serial.h>
#include <kernel/sys/config.h>
#include <kernel/time/manager.h>
#include <kernel/boot/phase.h>

namespace boot::logging {
    uint32_t init() {
        realloc_log_buffer(gKernelMessageSize * (uint32_t)gKernelConfiguration()->logsize.value);
        return 0;
    }
}

namespace {
    LogBuffer* gInitialRingBuffer() {
        static constexpr size_t gBufferSize = 32_KB;

        static char gMemory[gBufferSize] = {0};
        static LogBuffer gLogBuffer(&gMemory[0], gBufferSize);
        return &gLogBuffer;
    }

    log_stats_t& gLogStats() {
        static log_stats_t stats;

        return stats;
    }
}

LogBuffer *realloc_log_buffer(size_t size) {
    LOG_INFO("allocating a new kernel log - size = %u", size);
    LogBuffer *old_buffer = get_log_buffer();
    LogBuffer *new_buffer = new LogBuffer(size, *old_buffer);
    new_buffer = get_log_buffer(new_buffer);
    LOG_INFO("old kernel buffer at 0x%p; new buffer at 0x%p", old_buffer, new_buffer);
    return new_buffer;
}

LogBuffer* get_log_buffer(LogBuffer* buffer) {
    static LogBuffer *gBuffer = nullptr;
    if (buffer != nullptr) {
        gBuffer = buffer;
    }
    if (gBuffer == nullptr) {
        gBuffer = gInitialRingBuffer();
    }
    return gBuffer;
}

log_stats_t read_log_stats() {
    return gLogStats();
}

extern "C"
void __really_log(const char* tag, const char* filename, unsigned long line, const char* fmt, va_list args) {
    if (gKernelConfiguration()->logging.value == kernel_config_t::config_logging::gNoLogging) return;

    LogBuffer* the_log_buffer = get_log_buffer();
    log_stats_t& the_log_stats(gLogStats());

    static char gBuffer[gKernelMessageSize];
    size_t n = 0;
    const auto uptime = TimeManager::get().millisUptime();
    const auto uptime_sec = uptime / 1000;
    const uint32_t uptime_ms = uptime % 1000;
    n = sprint(&gBuffer[0], gKernelMessageSize, "[%llu.%u] %s:%lu ",
        uptime_sec, uptime_ms,
        filename, line);
    if (tag) n += sprint(&gBuffer[n], gKernelMessageSize-n, "(%s) ", tag);
	vsprint(&gBuffer[n], gKernelMessageSize-n, fmt, args);
    const auto& buflen = strlen(gBuffer) + 1;

    ++the_log_stats.num_log_entries;
    the_log_stats.total_log_size += buflen;
    if (buflen > the_log_stats.max_log_entry_size) {
        the_log_stats.max_log_entry_size = buflen;
    }

    the_log_buffer->write(gBuffer);
    the_log_buffer->write("\n");

    if (gKernelConfiguration()->logging.value == kernel_config_t::config_logging::gNoSerialLogging) return;
    Serial::get().write(gBuffer).write("\n");
}
