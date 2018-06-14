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

enum class LogLevel : uint8_t {
	DEBUG =   0x0,
	INFO =    0x1,
	WARNING = 0x2,
	ERROR  =  0x4,
};

void logimpl(const char* filename, size_t line, const char* msg, va_list args);

#ifndef LOG_LEVEL
#define LOG_LEVEL LogLevel::DEBUG
#endif

template<LogLevel level>
void log(const char* filename, size_t line, const char* msg, ...) {
	va_list args;
	va_start (args, msg);

	if (level >= LOG_LEVEL) {
		logimpl(filename, line, msg, args);
	}

	va_end(args);
}

#ifdef LOG_NODEBUG
#define LOG_DEBUG(msg, ...)
#else
#define LOG_DEBUG(msg, ...)   log<LogLevel::DEBUG>  (__FILE__, __LINE__, msg, ##__VA_ARGS__)
#endif

#define LOG_INFO(msg, ...)    log<LogLevel::INFO>   (__FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) log<LogLevel::WARNING>(__FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)   log<LogLevel::ERROR>  (__FILE__, __LINE__, msg, ##__VA_ARGS__)

#endif
