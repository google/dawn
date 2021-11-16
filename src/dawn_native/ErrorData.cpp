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

#include "dawn_native/ErrorData.h"

#include "dawn_native/Error.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    std::unique_ptr<ErrorData> ErrorData::Create(InternalErrorType type,
                                                 std::string message,
                                                 const char* file,
                                                 const char* function,
                                                 int line) {
        std::unique_ptr<ErrorData> error = std::make_unique<ErrorData>(type, message);
        error->AppendBacktrace(file, function, line);
        return error;
    }

    ErrorData::ErrorData(InternalErrorType type, std::string message)
        : mType(type), mMessage(std::move(message)) {
    }

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

    std::string ErrorData::GetFormattedMessage() const {
        std::ostringstream ss;
        ss << mMessage << "\n";

        if (!mContexts.empty()) {
            for (auto context : mContexts) {
                ss << " - While " << context << "\n";
            }
        }

        // For non-validation errors, or erros that lack a context include the
        // stack trace for debugging purposes.
        if (mContexts.empty() || mType != InternalErrorType::Validation) {
            for (const auto& callsite : mBacktrace) {
                ss << "    at " << callsite.function << " (" << callsite.file << ":"
                   << callsite.line << ")\n";
            }
        }

        if (!mDebugGroups.empty()) {
            ss << "\nDebug group stack:\n";
            for (auto label : mDebugGroups) {
                ss << " > \"" << label << "\"\n";
            }
        }

        return ss.str();
    }

}  // namespace dawn_native
