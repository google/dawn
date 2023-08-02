// Copyright 2018 The Dawn Authors
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

#include "dawn/native/ErrorData.h"

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/native/Error.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

std::unique_ptr<ErrorData> ErrorData::Create(InternalErrorType type,
                                             std::string message,
                                             const char* file,
                                             const char* function,
                                             int line) {
    std::unique_ptr<ErrorData> error = std::make_unique<ErrorData>(type, message);
    error->AppendBacktrace(file, function, line);

    auto [var, present] = GetEnvironmentVar("DAWN_DEBUG_BREAK_ON_ERROR");
    if (present && !var.empty() && var != "0") {
        ErrorLog() << error->GetMessage();
        BreakPoint();
    }
    return error;
}

ErrorData::ErrorData(InternalErrorType type, std::string message)
    : mType(type), mMessage(std::move(message)) {}

ErrorData::~ErrorData() = default;

void ErrorData::AppendBacktrace(const char* file, const char* function, int line) {
    BacktraceRecord record;
    record.file = file;
    record.function = function;
    record.line = line;

    mBacktrace.push_back(std::move(record));
}

void ErrorData::AppendContext(std::string context) {
    mContexts.push_back(std::move(context));
}

void ErrorData::AppendDebugGroup(std::string label) {
    mDebugGroups.push_back(std::move(label));
}

void ErrorData::AppendBackendMessage(std::string message) {
    mBackendMessages.push_back(std::move(message));
}

InternalErrorType ErrorData::GetType() const {
    return mType;
}

const std::string& ErrorData::GetMessage() const {
    return mMessage;
}

const std::vector<ErrorData::BacktraceRecord>& ErrorData::GetBacktrace() const {
    return mBacktrace;
}

const std::vector<std::string>& ErrorData::GetContexts() const {
    return mContexts;
}

const std::vector<std::string>& ErrorData::GetDebugGroups() const {
    return mDebugGroups;
}

const std::vector<std::string>& ErrorData::GetBackendMessages() const {
    return mBackendMessages;
}

std::string ErrorData::GetFormattedMessage() const {
    std::ostringstream ss;
    ss << mMessage << "\n";

    if (!mContexts.empty()) {
        for (auto context : mContexts) {
            ss << " - While " << context << "\n";
        }
    }

    // For non-validation errors, or errors that lack a context include the
    // stack trace for debugging purposes.
    if (mContexts.empty() || mType != InternalErrorType::Validation) {
        for (const auto& callsite : mBacktrace) {
            ss << "    at " << callsite.function << " (" << callsite.file << ":" << callsite.line
               << ")\n";
        }
    }

    if (!mDebugGroups.empty()) {
        ss << "\nDebug group stack:\n";
        for (auto label : mDebugGroups) {
            ss << " > \"" << label << "\"\n";
        }
    }

    if (!mBackendMessages.empty()) {
        ss << "\nBackend messages:\n";
        for (auto message : mBackendMessages) {
            ss << " * " << message << "\n";
        }
    }

    return ss.str();
}

}  // namespace dawn::native
