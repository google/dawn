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

#ifndef SRC_DAWN_NATIVE_COMPILATIONMESSAGES_H_
#define SRC_DAWN_NATIVE_COMPILATIONMESSAGES_H_

#include <string>
#include <vector>

#include "dawn/native/Error.h"
#include "dawn/native/dawn_platform.h"

#include "dawn/common/NonCopyable.h"

namespace tint::diag {
class Diagnostic;
class List;
}  // namespace tint::diag

namespace dawn::native {

ResultOrError<uint64_t> CountUTF16CodeUnitsFromUTF8String(const std::string_view& utf8String);

class OwnedCompilationMessages : public NonCopyable {
  public:
    OwnedCompilationMessages();
    ~OwnedCompilationMessages();

    void AddMessageForTesting(
        std::string message,
        wgpu::CompilationMessageType type = wgpu::CompilationMessageType::Info,
        uint64_t lineNum = 0,
        uint64_t linePos = 0,
        uint64_t offset = 0,
        uint64_t length = 0);
    MaybeError AddMessages(const tint::diag::List& diagnostics);
    void ClearMessages();

    const WGPUCompilationInfo* GetCompilationInfo();
    const std::vector<std::string>& GetFormattedTintMessages();

  private:
    MaybeError AddMessage(const tint::diag::Diagnostic& diagnostic);
    void AddFormattedTintMessages(const tint::diag::List& diagnostics);

    WGPUCompilationInfo mCompilationInfo;
    std::vector<std::string> mMessageStrings;
    std::vector<WGPUCompilationMessage> mMessages;
    std::vector<std::string> mFormattedTintMessages;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMPILATIONMESSAGES_H_
