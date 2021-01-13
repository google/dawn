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

#include "dawn_native/ErrorScope.h"

#include "common/Assert.h"

namespace dawn_native {

    ErrorScope::ErrorScope() : mIsRoot(true) {
    }

    ErrorScope::ErrorScope(wgpu::ErrorFilter errorFilter, ErrorScope* parent)
        : RefCounted(), mErrorFilter(errorFilter), mParent(parent), mIsRoot(false) {
        ASSERT(mParent != nullptr);
    }

    ErrorScope::~ErrorScope() {
        if (!IsRoot()) {
            RunNonRootCallback();
        }
    }

    void ErrorScope::SetCallback(wgpu::ErrorCallback callback, void* userdata) {
        mCallback = callback;
        mUserdata = userdata;
    }

    ErrorScope* ErrorScope::GetParent() {
        return mParent.Get();
    }

    bool ErrorScope::IsRoot() const {
        return mIsRoot;
    }

    void ErrorScope::RunNonRootCallback() {
        ASSERT(!IsRoot());

        if (mCallback != nullptr) {
            // For non-root error scopes, the callback can run at most once.
            mCallback(static_cast<WGPUErrorType>(mErrorType), mErrorMessage.c_str(), mUserdata);
            mCallback = nullptr;
        }
    }

    void ErrorScope::HandleError(wgpu::ErrorType type, const char* message) {
        HandleErrorImpl(this, type, message);
    }

    void ErrorScope::UnlinkForShutdown() {
        UnlinkForShutdownImpl(this);
    }

    // static
    void ErrorScope::HandleErrorImpl(ErrorScope* scope, wgpu::ErrorType type, const char* message) {
        ErrorScope* currentScope = scope;
        for (; !currentScope->IsRoot(); currentScope = currentScope->GetParent()) {
            ASSERT(currentScope != nullptr);

            bool consumed = false;
            switch (type) {
                case wgpu::ErrorType::Validation:
                    if (currentScope->mErrorFilter != wgpu::ErrorFilter::Validation) {
                        // Error filter does not match. Move on to the next scope.
                        continue;
                    }
                    consumed = true;
                    break;

                case wgpu::ErrorType::OutOfMemory:
                    if (currentScope->mErrorFilter != wgpu::ErrorFilter::OutOfMemory) {
                        // Error filter does not match. Move on to the next scope.
                        continue;
                    }
                    consumed = true;
                    break;

                // DeviceLost is fatal. All error scopes capture them.
                // |consumed| is false because these should bubble to all scopes.
                case wgpu::ErrorType::DeviceLost:
                    consumed = false;
                    if (currentScope->mErrorType != wgpu::ErrorType::DeviceLost) {
                        // DeviceLost overrides any other error that is not a DeviceLost.
                        currentScope->mErrorType = type;
                        currentScope->mErrorMessage = message;
                    }
                    break;

                case wgpu::ErrorType::Unknown:
                    // Means the scope was destroyed before contained work finished.
                    // This happens when you destroy the device while there's pending work.
                    // That's handled in ErrorScope::UnlinkForShutdownImpl, not here.
                case wgpu::ErrorType::NoError:
                    // Not considered an error, and should never be passed to HandleError.
                    UNREACHABLE();
                    return;
            }

            // Record the error if the scope doesn't have one yet.
            if (currentScope->mErrorType == wgpu::ErrorType::NoError) {
                currentScope->mErrorType = type;
                currentScope->mErrorMessage = message;
            }

            if (consumed) {
                return;
            }
        }

        // The root error scope captures all uncaptured errors.
        // Except, it should not capture device lost errors since those go to
        // the device lost callback.
        ASSERT(currentScope->IsRoot());
        if (currentScope->mCallback && type != wgpu::ErrorType::DeviceLost) {
            currentScope->mCallback(static_cast<WGPUErrorType>(type), message,
                                    currentScope->mUserdata);
        }
    }

    // static
    void ErrorScope::UnlinkForShutdownImpl(ErrorScope* scope) {
        Ref<ErrorScope> currentScope = scope;
        Ref<ErrorScope> parentScope = nullptr;
        for (; !currentScope->IsRoot(); currentScope = parentScope.Get()) {
            ASSERT(!currentScope->IsRoot());
            ASSERT(currentScope != nullptr);
            parentScope = std::move(currentScope->mParent);
            ASSERT(parentScope != nullptr);

            // On shutdown, error scopes that have yet to have a status get Unknown.
            if (currentScope->mErrorType == wgpu::ErrorType::NoError) {
                currentScope->mErrorType = wgpu::ErrorType::Unknown;
                currentScope->mErrorMessage = "Error scope destroyed";
            }

            // Run the callback if it hasn't run already.
            currentScope->RunNonRootCallback();
        }
    }

}  // namespace dawn_native
