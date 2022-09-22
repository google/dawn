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

#include "dawn/native/EncodingContext.h"

#include "dawn/common/Assert.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/IndirectDrawValidationEncoder.h"
#include "dawn/native/RenderBundleEncoder.h"

namespace dawn::native {

EncodingContext::EncodingContext(DeviceBase* device, const ApiObjectBase* initialEncoder)
    : mDevice(device),
      mTopLevelEncoder(initialEncoder),
      mCurrentEncoder(initialEncoder),
      mDestroyed(device->IsLost()) {}

EncodingContext::~EncodingContext() {
    Destroy();
}

void EncodingContext::Destroy() {
    if (mDestroyed) {
        return;
    }
    if (!mWereCommandsAcquired) {
        FreeCommands(GetIterator());
    }
    // If we weren't already finished, then we want to handle an error here so that any calls
    // to Finish after Destroy will return a meaningful error.
    if (!IsFinished()) {
        HandleError(DAWN_VALIDATION_ERROR("Destroyed encoder cannot be finished."));
    }
    mDestroyed = true;
    mCurrentEncoder = nullptr;
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
    CommitCommands(std::move(mPendingCommands));
    if (!mWasMovedToIterator) {
        mIterator.AcquireCommandBlocks(std::move(mAllocators));
        mWasMovedToIterator = true;
    }
}

void EncodingContext::HandleError(std::unique_ptr<ErrorData> error) {
    // Append in reverse so that the most recently set debug group is printed first, like a
    // call stack.
    for (auto iter = mDebugGroupLabels.rbegin(); iter != mDebugGroupLabels.rend(); ++iter) {
        error->AppendDebugGroup(*iter);
    }

    if (!IsFinished()) {
        // Encoding should only generate validation errors.
        ASSERT(error->GetType() == InternalErrorType::Validation);
        // If the encoding context is not finished, errors are deferred until
        // Finish() is called.
        if (mError == nullptr) {
            mError = std::move(error);
        }
    } else {
        mDevice->HandleError(error->GetType(), error->GetFormattedMessage().c_str());
    }
}

void EncodingContext::WillBeginRenderPass() {
    ASSERT(mCurrentEncoder == mTopLevelEncoder);
    if (mDevice->IsValidationEnabled() || mDevice->MayRequireDuplicationOfIndirectParameters()) {
        // When validation is enabled or indirect parameters require duplication, we are going
        // to want to capture all commands encoded between and including BeginRenderPassCmd and
        // EndRenderPassCmd, and defer their sequencing util after we have a chance to insert
        // any necessary validation or duplication commands. To support this we commit any
        // current commands now, so that the impending BeginRenderPassCmd starts in a fresh
        // CommandAllocator.
        CommitCommands(std::move(mPendingCommands));
    }
}

void EncodingContext::EnterPass(const ApiObjectBase* passEncoder) {
    // Assert we're at the top level.
    ASSERT(mCurrentEncoder == mTopLevelEncoder);
    ASSERT(passEncoder != nullptr);

    mCurrentEncoder = passEncoder;
}

MaybeError EncodingContext::ExitRenderPass(const ApiObjectBase* passEncoder,
                                           RenderPassResourceUsageTracker usageTracker,
                                           CommandEncoder* commandEncoder,
                                           IndirectDrawMetadata indirectDrawMetadata) {
    ASSERT(mCurrentEncoder != mTopLevelEncoder);
    ASSERT(mCurrentEncoder == passEncoder);

    mCurrentEncoder = mTopLevelEncoder;

    if (mDevice->IsValidationEnabled() || mDevice->MayRequireDuplicationOfIndirectParameters()) {
        // With validation enabled, commands were committed just before BeginRenderPassCmd was
        // encoded by our RenderPassEncoder (see WillBeginRenderPass above). This means
        // mPendingCommands contains only the commands from BeginRenderPassCmd to
        // EndRenderPassCmd, inclusive. Now we swap out this allocator with a fresh one to give
        // the validation encoder a chance to insert its commands first.
        // Note: If encoding validation commands fails, no commands should be in mPendingCommands,
        //       so swap back the renderCommands to ensure that they are not leaked.
        CommandAllocator renderCommands = std::move(mPendingCommands);
        DAWN_TRY_WITH_CLEANUP(EncodeIndirectDrawValidationCommands(
                                  mDevice, commandEncoder, &usageTracker, &indirectDrawMetadata),
                              { mPendingCommands = std::move(renderCommands); });
        CommitCommands(std::move(mPendingCommands));
        CommitCommands(std::move(renderCommands));
    }

    mRenderPassUsages.push_back(usageTracker.AcquireResourceUsage());
    return {};
}

void EncodingContext::ExitComputePass(const ApiObjectBase* passEncoder,
                                      ComputePassResourceUsage usages) {
    ASSERT(mCurrentEncoder != mTopLevelEncoder);
    ASSERT(mCurrentEncoder == passEncoder);

    mCurrentEncoder = mTopLevelEncoder;
    mComputePassUsages.push_back(std::move(usages));
}

void EncodingContext::EnsurePassExited(const ApiObjectBase* passEncoder) {
    if (mCurrentEncoder != mTopLevelEncoder && mCurrentEncoder == passEncoder) {
        // The current pass encoder is being deleted. Implicitly end the pass with an error.
        mCurrentEncoder = mTopLevelEncoder;
        HandleError(DAWN_VALIDATION_ERROR("Command buffer recording ended before %s was ended.",
                                          passEncoder));
    }
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

void EncodingContext::PushDebugGroupLabel(const char* groupLabel) {
    mDebugGroupLabels.emplace_back(groupLabel);
}

void EncodingContext::PopDebugGroupLabel() {
    mDebugGroupLabels.pop_back();
}

MaybeError EncodingContext::Finish() {
    DAWN_INVALID_IF(IsFinished(), "Command encoding already finished.");

    const ApiObjectBase* currentEncoder = mCurrentEncoder;
    const ApiObjectBase* topLevelEncoder = mTopLevelEncoder;

    // Even if finish validation fails, it is now invalid to call any encoding commands,
    // so we clear the encoders. Note: mTopLevelEncoder == nullptr is used as a flag for
    // if Finish() has been called.
    mCurrentEncoder = nullptr;
    mTopLevelEncoder = nullptr;
    CommitCommands(std::move(mPendingCommands));

    if (mError != nullptr) {
        return std::move(mError);
    }
    DAWN_INVALID_IF(currentEncoder != topLevelEncoder,
                    "Command buffer recording ended before %s was ended.", currentEncoder);
    return {};
}

void EncodingContext::CommitCommands(CommandAllocator allocator) {
    if (!allocator.IsEmpty()) {
        mAllocators.push_back(std::move(allocator));
    }
}

bool EncodingContext::IsFinished() const {
    return mTopLevelEncoder == nullptr;
}

}  // namespace dawn::native
