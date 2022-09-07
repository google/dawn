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

#include "dawn/native/ErrorScope.h"

#include <utility>

#include "dawn/common/Assert.h"

namespace dawn::native {

namespace {

wgpu::ErrorType ErrorFilterToErrorType(wgpu::ErrorFilter filter) {
    switch (filter) {
        case wgpu::ErrorFilter::Validation:
            return wgpu::ErrorType::Validation;
        case wgpu::ErrorFilter::OutOfMemory:
            return wgpu::ErrorType::OutOfMemory;
        case wgpu::ErrorFilter::Internal:
            return wgpu::ErrorType::Internal;
    }
    UNREACHABLE();
}

}  // namespace

ErrorScope::ErrorScope(wgpu::ErrorFilter errorFilter)
    : mMatchedErrorType(ErrorFilterToErrorType(errorFilter)) {}

wgpu::ErrorType ErrorScope::GetErrorType() const {
    return mCapturedError;
}

const char* ErrorScope::GetErrorMessage() const {
    return mErrorMessage.c_str();
}

ErrorScopeStack::ErrorScopeStack() = default;

ErrorScopeStack::~ErrorScopeStack() = default;

void ErrorScopeStack::Push(wgpu::ErrorFilter filter) {
    mScopes.push_back(ErrorScope(filter));
}

ErrorScope ErrorScopeStack::Pop() {
    ASSERT(!mScopes.empty());
    ErrorScope scope = std::move(mScopes.back());
    mScopes.pop_back();
    return scope;
}

bool ErrorScopeStack::Empty() const {
    return mScopes.empty();
}

bool ErrorScopeStack::HandleError(wgpu::ErrorType type, const char* message) {
    for (auto it = mScopes.rbegin(); it != mScopes.rend(); ++it) {
        if (it->mMatchedErrorType != type) {
            // Error filter does not match. Move on to the next scope.
            continue;
        }

        // Filter matches.
        // Record the error if the scope doesn't have one yet.
        if (it->mCapturedError == wgpu::ErrorType::NoError) {
            it->mCapturedError = type;
            it->mErrorMessage = message;
        }

        if (type == wgpu::ErrorType::DeviceLost) {
            if (it->mCapturedError != wgpu::ErrorType::DeviceLost) {
                // DeviceLost overrides any other error that is not a DeviceLost.
                it->mCapturedError = type;
                it->mErrorMessage = message;
            }
        } else {
            // Errors that are not device lost are captured and stop propogating.
            return true;
        }
    }

    // The error was not captured.
    return false;
}

}  // namespace dawn::native
