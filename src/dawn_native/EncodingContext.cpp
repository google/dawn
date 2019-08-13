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

#include "dawn_native/EncodingContext.h"

#include "common/Assert.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/ErrorData.h"

namespace dawn_native {

    EncodingContext::EncodingContext(DeviceBase* device, const ObjectBase* initialEncoder)
        : mDevice(device), mTopLevelEncoder(initialEncoder), mCurrentEncoder(initialEncoder) {
    }

    EncodingContext::~EncodingContext() {
        if (!mWereCommandsAcquired) {
            FreeCommands(GetIterator());
        }
    }

    CommandIterator EncodingContext::AcquireCommands() {
        ASSERT(!mWereCommandsAcquired);
        mWereCommandsAcquired = true;
        return std::move(mIterator);
    }

    CommandIterator* EncodingContext::GetIterator() {
        if (!mWasMovedToIterator) {
            mIterator = std::move(mAllocator);
            mWasMovedToIterator = true;
        }
        return &mIterator;
    }

    void EncodingContext::HandleError(const char* message) {
        if (!IsFinished()) {
            // If the encoding context is not finished, errors are deferred until
            // Finish() is called.
            if (!mGotError) {
                mGotError = true;
                mErrorMessage = message;
            }
        } else {
            mDevice->HandleError(message);
        }
    }

    void EncodingContext::EnterPass(const ObjectBase* passEncoder) {
        // Assert we're at the top level.
        ASSERT(mCurrentEncoder == mTopLevelEncoder);
        ASSERT(passEncoder != nullptr);

        mCurrentEncoder = passEncoder;
    }

    void EncodingContext::ExitPass(const ObjectBase* passEncoder) {
        // Assert we're not at the top level.
        ASSERT(mCurrentEncoder != mTopLevelEncoder);
        // Assert the pass encoder is current.
        ASSERT(mCurrentEncoder == passEncoder);

        mCurrentEncoder = mTopLevelEncoder;
    }

    MaybeError EncodingContext::Finish() {
        if (IsFinished()) {
            return DAWN_VALIDATION_ERROR("Command encoding already finished");
        }

        const void* currentEncoder = mCurrentEncoder;
        const void* topLevelEncoder = mTopLevelEncoder;

        // Even if finish validation fails, it is now invalid to call any encoding commands,
        // so we clear the encoders. Note: mTopLevelEncoder == nullptr is used as a flag for
        // if Finish() has been called.
        mCurrentEncoder = nullptr;
        mTopLevelEncoder = nullptr;

        if (mGotError) {
            return DAWN_VALIDATION_ERROR(mErrorMessage);
        }
        if (currentEncoder != topLevelEncoder) {
            return DAWN_VALIDATION_ERROR("Command buffer recording ended mid-pass");
        }
        return {};
    }

    bool EncodingContext::IsFinished() const {
        return mTopLevelEncoder == nullptr;
    }

}  // namespace dawn_native
