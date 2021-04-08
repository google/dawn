// Copyright 2021 The Dawn Authors
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

#include "dawn_native/CompilationMessages.h"

#include "common/Assert.h"
#include "dawn_native/dawn_platform.h"

#include <tint/tint.h>

namespace dawn_native {

    namespace {

        WGPUCompilationMessageType tintSeverityToMessageType(tint::diag::Severity severity) {
            switch (severity) {
                case tint::diag::Severity::Note:
                    return WGPUCompilationMessageType_Info;
                case tint::diag::Severity::Warning:
                    return WGPUCompilationMessageType_Warning;
                default:
                    return WGPUCompilationMessageType_Error;
            }
        }

    }  // anonymous namespace

    OwnedCompilationMessages::OwnedCompilationMessages() {
        mCompilationInfo.messageCount = 0;
        mCompilationInfo.messages = nullptr;
    }

    void OwnedCompilationMessages::AddMessage(std::string message,
                                              wgpu::CompilationMessageType type,
                                              uint64_t lineNum,
                                              uint64_t linePos) {
        // Cannot add messages after GetCompilationInfo has been called.
        ASSERT(mCompilationInfo.messages == nullptr);

        mMessageStrings.push_back(message);
        mMessages.push_back(
            {nullptr, static_cast<WGPUCompilationMessageType>(type), lineNum, linePos});
    }

    void OwnedCompilationMessages::AddMessage(const tint::diag::Diagnostic& diagnostic) {
        // Cannot add messages after GetCompilationInfo has been called.
        ASSERT(mCompilationInfo.messages == nullptr);

        if (diagnostic.code) {
            mMessageStrings.push_back(std::string(diagnostic.code) + ": " + diagnostic.message);
        } else {
            mMessageStrings.push_back(diagnostic.message);
        }
        mMessages.push_back({nullptr, tintSeverityToMessageType(diagnostic.severity),
                             diagnostic.source.range.begin.line,
                             diagnostic.source.range.begin.column});
    }

    void OwnedCompilationMessages::AddMessages(const tint::diag::List& diagnostics) {
        // Cannot add messages after GetCompilationInfo has been called.
        ASSERT(mCompilationInfo.messages == nullptr);

        for (const auto& diag : diagnostics) {
            AddMessage(diag);
        }
    }

    void OwnedCompilationMessages::ClearMessages() {
        // Cannot clear messages after GetCompilationInfo has been called.
        ASSERT(mCompilationInfo.messages == nullptr);

        mMessageStrings.clear();
        mMessages.clear();
    }

    const WGPUCompilationInfo* OwnedCompilationMessages::GetCompilationInfo() {
        mCompilationInfo.messageCount = mMessages.size();
        mCompilationInfo.messages = mMessages.data();

        // Ensure every message points at the correct message string. Cannot do this earlier, since
        // vector reallocations may move the pointers around.
        for (size_t i = 0; i < mCompilationInfo.messageCount; ++i) {
            WGPUCompilationMessage& message = mMessages[i];
            std::string& messageString = mMessageStrings[i];
            message.message = messageString.c_str();
        }

        return &mCompilationInfo;
    }

}  // namespace dawn_native
