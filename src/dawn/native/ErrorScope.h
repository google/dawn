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

#ifndef SRC_DAWN_NATIVE_ERRORSCOPE_H_
#define SRC_DAWN_NATIVE_ERRORSCOPE_H_

#include <string>
#include <vector>

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class ErrorScope {
  public:
    wgpu::ErrorType GetErrorType() const;
    const char* GetErrorMessage() const;

  private:
    friend class ErrorScopeStack;
    explicit ErrorScope(wgpu::ErrorFilter errorFilter);

    wgpu::ErrorType mMatchedErrorType;
    wgpu::ErrorType mCapturedError = wgpu::ErrorType::NoError;
    std::string mErrorMessage = "";
};

class ErrorScopeStack {
  public:
    ErrorScopeStack();
    ~ErrorScopeStack();

    void Push(wgpu::ErrorFilter errorFilter);
    ErrorScope Pop();

    bool Empty() const;

    // Pass an error to the scopes in the stack. Returns true if one of the scopes
    // captured the error. Returns false if the error should be forwarded to the
    // uncaptured error callback.
    bool HandleError(wgpu::ErrorType type, const char* message);

  private:
    std::vector<ErrorScope> mScopes;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ERRORSCOPE_H_
