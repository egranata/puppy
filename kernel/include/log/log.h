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

#ifndef LOG_LOG
#define LOG_LOG

#include <stdarg.h>
#include <kernel/sys/stdint.h>
#include <kernel/libc/enableif.h>
#include <kernel/libc/ringbuffer.h>

#ifdef LOG_NODEBUG
#warning define a proper logging level instead of relying on LOG_NODEBUG
#define LOG_LEVEL 1
#endif

using LogBuffer = RingBuffer<char>;

LogBuffer* get_log_buffer(LogBuffer* buffer = nullptr);

extern "C"
void __really_log(const char* tag, const char* filename, unsigned long line, const char* fmt, va_list args);

#define LOG_TAG(NAME,LEVEL) \
__attribute__((unused)) static constexpr uint8_t g ## NAME ## LogLevel = LEVEL

#define TAG_DEBUG(TAG,...) log_debug<g ## TAG ## LogLevel>(#TAG, __FILE__, __LINE__, __VA_ARGS__)
#define TAG_INFO(TAG,...) log_info<g ## TAG ## LogLevel>(#TAG, __FILE__, __LINE__, __VA_ARGS__)
#define TAG_WARNING(TAG,...) log_warning<g ## TAG ## LogLevel>(#TAG, __FILE__, __LINE__, __VA_ARGS__)
#define TAG_ERROR(TAG,...) log_error<g ## TAG ## LogLevel>(#TAG, __FILE__, __LINE__, __VA_ARGS__)

#ifndef LOG_LEVEL
#ifdef NDEBUG
static constexpr uint8_t gLogLevel = 2;
#else
static constexpr uint8_t gLogLevel = 0;
#endif
#else
static constexpr uint8_t gLogLevel = LOG_LEVEL;
#endif

#define LOG_DEBUG(...) log_debug(nullptr, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) log_info(nullptr, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...) log_warning(nullptr, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_error(nullptr, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_ENTRY(level, value) \
template<uint8_t LL = gLogLevel> \
static typename enable_if<(LL <= value), void>::type \
log_ ## level (const char* tag, const char* file, int line, const char* fmt, ...) { \
    va_list ap; \
    va_start(ap, fmt); \
    __really_log(tag, file, line, fmt, ap); \
    va_end(ap); \
} \
template<uint8_t LL = gLogLevel> \
static typename enable_if<(LL > value), void>::type \
log_ ## level (const char*, const char*, int, const char*, ...) {}

LOG_ENTRY(debug, 0)
LOG_ENTRY(info, 1)
LOG_ENTRY(warning, 2)
LOG_ENTRY(error, 3)

#endif
