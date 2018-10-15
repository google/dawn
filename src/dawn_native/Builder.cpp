// Copyright 2017 The Dawn Authors
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

#include "dawn_native/Builder.h"

#include "common/Assert.h"
#include "dawn_native/Device.h"

namespace dawn_native {

    bool BuilderBase::CanBeUsed() const {
        return !mIsConsumed && !mGotStatus;
    }

    void BuilderBase::HandleError(const char* message) {
        SetStatus(dawn::BuilderErrorStatus::Error, message);
    }

    void BuilderBase::SetErrorCallback(dawn::BuilderErrorCallback callback,
                                       dawn::CallbackUserdata userdata1,
                                       dawn::CallbackUserdata userdata2) {
        mCallback = callback;
        mUserdata1 = userdata1;
        mUserdata2 = userdata2;
    }

    BuilderBase::BuilderBase(DeviceBase* device) : ObjectBase(device) {
    }

    BuilderBase::~BuilderBase() {
        if (!mIsConsumed && mCallback != nullptr) {
            mCallback(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, "Builder destroyed before GetResult",
                      mUserdata1, mUserdata2);
        }
    }

    void BuilderBase::SetStatus(dawn::BuilderErrorStatus status, const char* message) {
        ASSERT(status != dawn::BuilderErrorStatus::Success);
        ASSERT(status != dawn::BuilderErrorStatus::Unknown);
        ASSERT(!mGotStatus);  // This is not strictly necessary but something to strive for.
        mGotStatus = true;

        mStoredStatus = status;
        mStoredMessage = message;
    }

    bool BuilderBase::HandleResult(ObjectBase* result) {
        // GetResult can only be called once.
        ASSERT(!mIsConsumed);
        mIsConsumed = true;

        // result == nullptr implies there was an error which implies we should have a status set.
        ASSERT(result != nullptr || mGotStatus);

        // If we have any error, then we have to return nullptr
        if (mGotStatus) {
            ASSERT(mStoredStatus != dawn::BuilderErrorStatus::Success);

            // The application will never see "result" so we need to remove the
            // external ref here.
            if (result != nullptr) {
                result->Release();
                result = nullptr;
            }

            // Unhandled builder errors are promoted to device errors
            if (!mCallback)
                GetDevice()->HandleError(("Unhandled builder error: " + mStoredMessage).c_str());
        } else {
            ASSERT(mStoredStatus == dawn::BuilderErrorStatus::Success);
            ASSERT(mStoredMessage.empty());
        }

        if (mCallback != nullptr) {
            mCallback(static_cast<dawnBuilderErrorStatus>(mStoredStatus), mStoredMessage.c_str(),
                      mUserdata1, mUserdata2);
        }

        return result != nullptr;
    }

}  // namespace dawn_native
