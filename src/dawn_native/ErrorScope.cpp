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
#include "dawn_native/ErrorData.h"

namespace dawn_native {

    ErrorScope::ErrorScope() = default;

    ErrorScope::ErrorScope(dawn::ErrorFilter errorFilter, ErrorScope* parent)
        : RefCounted(), mErrorFilter(errorFilter), mParent(parent) {
        ASSERT(mParent.Get() != nullptr);
    }

    ErrorScope::~ErrorScope() {
        if (mCallback == nullptr || IsRoot()) {
            return;
        }
        mCallback(static_cast<DawnErrorType>(mErrorType), mErrorMessage.c_str(), mUserdata);
    }

    void ErrorScope::SetCallback(dawn::ErrorCallback callback, void* userdata) {
        mCallback = callback;
        mUserdata = userdata;
    }

    ErrorScope* ErrorScope::GetParent() {
        return mParent.Get();
    }

    bool ErrorScope::IsRoot() const {
        return mParent.Get() == nullptr;
    }

    void ErrorScope::HandleError(dawn::ErrorType type, const char* message) {
        HandleErrorImpl(this, type, message);
    }

    void ErrorScope::HandleError(ErrorData* error) {
        ASSERT(error != nullptr);
        HandleErrorImpl(this, error->GetType(), error->GetMessage().c_str());
    }

    // static
    void ErrorScope::HandleErrorImpl(ErrorScope* scope, dawn::ErrorType type, const char* message) {
        ErrorScope* currentScope = scope;
        for (; !currentScope->IsRoot(); currentScope = currentScope->GetParent()) {
            ASSERT(currentScope != nullptr);

            bool consumed = false;
            switch (type) {
                case dawn::ErrorType::Validation:
                    if (currentScope->mErrorFilter != dawn::ErrorFilter::Validation) {
                        // Error filter does not match. Move on to the next scope.
                        continue;
                    }
                    consumed = true;
                    break;

                case dawn::ErrorType::OutOfMemory:
                    if (currentScope->mErrorFilter != dawn::ErrorFilter::OutOfMemory) {
                        // Error filter does not match. Move on to the next scope.
                        continue;
                    }
                    consumed = true;
                    break;

                // Unknown and DeviceLost are fatal. All error scopes capture them.
                // |consumed| is false because these should bubble to all scopes.
                case dawn::ErrorType::Unknown:
                case dawn::ErrorType::DeviceLost:
                    consumed = false;
                    break;

                case dawn::ErrorType::NoError:
                default:
                    UNREACHABLE();
                    return;
            }

            // Record the error if the scope doesn't have one yet.
            if (currentScope->mErrorType == dawn::ErrorType::NoError) {
                currentScope->mErrorType = type;
                currentScope->mErrorMessage = message;
            }

            if (consumed) {
                return;
            }
        }

        // The root error scope captures all uncaptured errors.
        ASSERT(currentScope->IsRoot());
        if (currentScope->mCallback) {
            currentScope->mCallback(static_cast<DawnErrorType>(type), message,
                                    currentScope->mUserdata);
        }
    }

    void ErrorScope::Destroy() {
        if (!IsRoot()) {
            mErrorType = dawn::ErrorType::Unknown;
            mErrorMessage = "Error scope destroyed";
        }
    }

}  // namespace dawn_native
