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

#ifndef SRC_DAWN_NATIVE_ERRORDATA_H_
#define SRC_DAWN_NATIVE_ERRORDATA_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "dawn/common/Compiler.h"

namespace wgpu {
enum class ErrorType : uint32_t;
}

namespace dawn {
using ErrorType = wgpu::ErrorType;
}

namespace dawn::native {
enum class InternalErrorType : uint32_t;

class [[nodiscard]] ErrorData {
  public:
    [[nodiscard]] static std::unique_ptr<ErrorData> Create(InternalErrorType type,
                                                           std::string message,
                                                           const char* file,
                                                           const char* function,
                                                           int line);
    ErrorData(InternalErrorType type, std::string message);
    ~ErrorData();

    struct BacktraceRecord {
        const char* file;
        const char* function;
        int line;
    };
    void AppendBacktrace(const char* file, const char* function, int line);
    void AppendContext(std::string context);
    void AppendDebugGroup(std::string label);
    void AppendBackendMessage(std::string message);

    InternalErrorType GetType() const;
    const std::string& GetMessage() const;
    const std::vector<BacktraceRecord>& GetBacktrace() const;
    const std::vector<std::string>& GetContexts() const;
    const std::vector<std::string>& GetDebugGroups() const;
    const std::vector<std::string>& GetBackendMessages() const;

    std::string GetFormattedMessage() const;

  private:
    InternalErrorType mType;
    std::string mMessage;
    std::vector<BacktraceRecord> mBacktrace;
    std::vector<std::string> mContexts;
    std::vector<std::string> mDebugGroups;
    std::vector<std::string> mBackendMessages;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ERRORDATA_H_
