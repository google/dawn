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
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/RenderBundleEncoder.h"

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
        MoveToIterator();
        ASSERT(!mWereCommandsAcquired);
        mWereCommandsAcquired = true;
        return std::move(mIterator);
    }

    CommandIterator* EncodingContext::GetIterator() {
        MoveToIterator();
        ASSERT(!mWereCommandsAcquired);
        return &mIterator;
    }

    void EncodingContext::MoveToIterator() {
        if (!mWasMovedToIterator) {
            mIterator = std::move(mAllocator);
            mWasMovedToIterator = true;
        }
    }

    void EncodingContext::HandleError(std::unique_ptr<ErrorData> error) {
        if (!IsFinished()) {
            // Encoding should only generate validation errors.
            ASSERT(error->GetType() == InternalErrorType::Validation);
            // If the encoding context is not finished, errors are deferred until
            // Finish() is called.
            if (mError == nullptr) {
                mError = std::move(error);
            }
        } else {
            mDevice->HandleError(error->GetType(), error->GetMessage().c_str());
        }
    }

    void EncodingContext::EnterPass(const ObjectBase* passEncoder) {
        // Assert we're at the top level.
        ASSERT(mCurrentEncoder == mTopLevelEncoder);
        ASSERT(passEncoder != nullptr);

        mCurrentEncoder = passEncoder;
    }

    void EncodingContext::ExitPass(const ObjectBase* passEncoder, RenderPassResourceUsage usages) {
        ASSERT(mCurrentEncoder != mTopLevelEncoder);
        ASSERT(mCurrentEncoder == passEncoder);

        mCurrentEncoder = mTopLevelEncoder;
        mRenderPassUsages.push_back(std::move(usages));
    }

    void EncodingContext::ExitPass(const ObjectBase* passEncoder, ComputePassResourceUsage usages) {
        ASSERT(mCurrentEncoder != mTopLevelEncoder);
        ASSERT(mCurrentEncoder == passEncoder);

        mCurrentEncoder = mTopLevelEncoder;
        mComputePassUsages.push_back(std::move(usages));
    }

    const RenderPassUsages& EncodingContext::GetRenderPassUsages() const {
        ASSERT(!mWereRenderPassUsagesAcquired);
        return mRenderPassUsages;
    }

    RenderPassUsages EncodingContext::AcquireRenderPassUsages() {
        ASSERT(!mWereRenderPassUsagesAcquired);
        mWereRenderPassUsagesAcquired = true;
        return std::move(mRenderPassUsages);
    }

    const ComputePassUsages& EncodingContext::GetComputePassUsages() const {
        ASSERT(!mWereComputePassUsagesAcquired);
        return mComputePassUsages;
    }

    ComputePassUsages EncodingContext::AcquireComputePassUsages() {
        ASSERT(!mWereComputePassUsagesAcquired);
        mWereComputePassUsagesAcquired = true;
        return std::move(mComputePassUsages);
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

        if (mError != nullptr) {
            return std::move(mError);
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
