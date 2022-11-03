// Copyright 2020 The Dawn Authors
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

#include "dawn/native/metal/CommandRecordingContext.h"

#include "dawn/common/Assert.h"

namespace dawn::native::metal {

CommandRecordingContext::CommandRecordingContext() = default;

CommandRecordingContext::~CommandRecordingContext() {
    // Commands must be acquired.
    ASSERT(mCommands == nullptr);
}

id<MTLCommandBuffer> CommandRecordingContext::GetCommands() {
    return mCommands.Get();
}

void CommandRecordingContext::SetNeedsSubmit() {
    mNeedsSubmit = true;
}
bool CommandRecordingContext::NeedsSubmit() const {
    return mNeedsSubmit;
}

void CommandRecordingContext::MarkUsed() {
    mUsed = true;
}
bool CommandRecordingContext::WasUsed() const {
    return mUsed;
}

MaybeError CommandRecordingContext::PrepareNextCommandBuffer(id<MTLCommandQueue> queue) {
    ASSERT(mCommands == nil);
    ASSERT(!mNeedsSubmit);
    ASSERT(!mUsed);

    // The MTLCommandBuffer will be autoreleased by default.
    // The autorelease pool may drain before the command buffer is submitted. Retain so it stays
    // alive.
    mCommands = AcquireNSPRef([[queue commandBuffer] retain]);
    if (mCommands == nil) {
        return DAWN_INTERNAL_ERROR("Failed to allocate an MTLCommandBuffer");
    }

    return {};
}

NSPRef<id<MTLCommandBuffer>> CommandRecordingContext::AcquireCommands() {
    // A blit encoder can be left open from WriteBuffer, make sure we close it.
    if (mCommands != nullptr) {
        EndBlit();
    }

    ASSERT(!mInEncoder);
    mNeedsSubmit = false;
    mUsed = false;
    return std::move(mCommands);
}

id<MTLBlitCommandEncoder> CommandRecordingContext::BeginBlit(MTLBlitPassDescriptor* descriptor)
    API_AVAILABLE(macos(11.0), ios(14.0)) {
    ASSERT(descriptor);
    ASSERT(mCommands != nullptr);
    ASSERT(mBlit == nullptr);
    ASSERT(!mInEncoder);

    mInEncoder = true;
    // The encoder is created autoreleased. Retain it to avoid the autoreleasepool from
    // draining from under us.
    mBlit.Acquire([[*mCommands blitCommandEncoderWithDescriptor:descriptor] retain]);
    return mBlit.Get();
}

id<MTLBlitCommandEncoder> CommandRecordingContext::EnsureBlit() {
    ASSERT(mCommands != nullptr);

    if (mBlit == nullptr) {
        ASSERT(!mInEncoder);
        mInEncoder = true;

        // The encoder is created autoreleased. Retain it to avoid the autoreleasepool from
        // draining from under us.
        mBlit.Acquire([[*mCommands blitCommandEncoder] retain]);
    }
    return mBlit.Get();
}

void CommandRecordingContext::EndBlit() {
    ASSERT(mCommands != nullptr);

    if (mBlit != nullptr) {
        [*mBlit endEncoding];
        mBlit = nullptr;
        mInEncoder = false;
    }
}

id<MTLComputeCommandEncoder> CommandRecordingContext::BeginCompute() {
    ASSERT(mCommands != nullptr);
    ASSERT(mCompute == nullptr);
    ASSERT(!mInEncoder);

    mInEncoder = true;
    // The encoder is created autoreleased. Retain it to avoid the autoreleasepool from
    // draining from under us.
    mCompute.Acquire([[*mCommands computeCommandEncoder] retain]);
    return mCompute.Get();
}

id<MTLComputeCommandEncoder> CommandRecordingContext::BeginCompute(
    MTLComputePassDescriptor* descriptor) API_AVAILABLE(macos(11.0), ios(14.0)) {
    ASSERT(descriptor);
    ASSERT(mCommands != nullptr);
    ASSERT(mCompute == nullptr);
    ASSERT(!mInEncoder);

    mInEncoder = true;
    // The encoder is created autoreleased. Retain it to avoid the autoreleasepool from
    // draining from under us.
    mCompute.Acquire([[*mCommands computeCommandEncoderWithDescriptor:descriptor] retain]);
    return mCompute.Get();
}

void CommandRecordingContext::EndCompute() {
    ASSERT(mCommands != nullptr);
    ASSERT(mCompute != nullptr);

    [*mCompute endEncoding];
    mCompute = nullptr;
    mInEncoder = false;
}

id<MTLRenderCommandEncoder> CommandRecordingContext::BeginRender(
    MTLRenderPassDescriptor* descriptor) {
    ASSERT(mCommands != nullptr);
    ASSERT(mRender == nullptr);
    ASSERT(!mInEncoder);

    mInEncoder = true;
    // The encoder is created autoreleased. Retain it to avoid the autoreleasepool from
    // draining from under us.
    mRender.Acquire([[*mCommands renderCommandEncoderWithDescriptor:descriptor] retain]);
    return mRender.Get();
}

void CommandRecordingContext::EndRender() {
    ASSERT(mCommands != nullptr);
    ASSERT(mRender != nullptr);

    [*mRender endEncoding];
    mRender = nullptr;
    mInEncoder = false;
}

}  // namespace dawn::native::metal
