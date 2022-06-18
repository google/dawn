// Copyright 2019 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn/common/Log.h"

#include <cstdio>
#include <string>

#include "dawn/common/Assert.h"
#include "dawn/common/Platform.h"

#if DAWN_PLATFORM_IS(ANDROID)
#include <android/log.h>
#endif

namespace dawn {

namespace {

#if !defined(DAWN_DISABLE_LOGGING)
const char* SeverityName(LogSeverity severity) {
    switch (severity) {
        case LogSeverity::Debug:
            return "Debug";
        case LogSeverity::Info:
            return "Info";
        case LogSeverity::Warning:
            return "Warning";
        case LogSeverity::Error:
            return "Error";
        default:
            UNREACHABLE();
            return "";
    }
}
#endif

#if DAWN_PLATFORM_IS(ANDROID)
android_LogPriority AndroidLogPriority(LogSeverity severity) {
    switch (severity) {
        case LogSeverity::Debug:
            return ANDROID_LOG_INFO;
        case LogSeverity::Info:
            return ANDROID_LOG_INFO;
        case LogSeverity::Warning:
            return ANDROID_LOG_WARN;
        case LogSeverity::Error:
            return ANDROID_LOG_ERROR;
        default:
            UNREACHABLE();
            return ANDROID_LOG_ERROR;
    }
}
#endif  // DAWN_PLATFORM_IS(ANDROID)

}  // anonymous namespace

LogMessage::LogMessage(LogSeverity severity) : mSeverity(severity) {}

LogMessage::LogMessage(LogMessage&& other) = default;

LogMessage& LogMessage::operator=(LogMessage&& other) = default;

#if defined(DAWN_DISABLE_LOGGING)
LogMessage::~LogMessage() {
    (void)mSeverity;
    // Don't print logs to make fuzzing more efficient. Implemented as
    // an early return to avoid warnings about unused member variables.
    return;
}
#else  // defined(DAWN_DISABLE_LOGGING)
LogMessage::~LogMessage() {
    std::string fullMessage = mStream.str();

    // If this message has been moved, its stream is empty.
    if (fullMessage.empty()) {
        return;
    }

    const char* severityName = SeverityName(mSeverity);

#if DAWN_PLATFORM_IS(ANDROID)
    android_LogPriority androidPriority = AndroidLogPriority(mSeverity);
    __android_log_print(androidPriority, "Dawn", "%s: %s\n", severityName, fullMessage.c_str());
#else   // DAWN_PLATFORM_IS(ANDROID)
    FILE* outputStream = stdout;
    if (mSeverity == LogSeverity::Warning || mSeverity == LogSeverity::Error) {
        outputStream = stderr;
    }

    // Note: we use fprintf because <iostream> includes static initializers.
    fprintf(outputStream, "%s: %s\n", severityName, fullMessage.c_str());
    fflush(outputStream);
#endif  // DAWN_PLATFORM_IS(ANDROID)
}
#endif  // defined(DAWN_DISABLE_LOGGING)

LogMessage DebugLog() {
    return LogMessage(LogSeverity::Debug);
}

LogMessage InfoLog() {
    return LogMessage(LogSeverity::Info);
}

LogMessage WarningLog() {
    return LogMessage(LogSeverity::Warning);
}

LogMessage ErrorLog() {
    return LogMessage(LogSeverity::Error);
}

LogMessage DebugLog(const char* file, const char* function, int line) {
    LogMessage message = DebugLog();
    message << file << ":" << line << "(" << function << ")";
    return message;
}

}  // namespace dawn
