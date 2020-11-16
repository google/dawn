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

#include "dawn_native/metal/CommandRecordingContext.h"

#include "common/Assert.h"

namespace dawn_native { namespace metal {

    CommandRecordingContext::CommandRecordingContext() = default;

    CommandRecordingContext::CommandRecordingContext(NSPRef<id<MTLCommandBuffer>> commands)
        : mCommands(std::move(commands)) {
    }

    CommandRecordingContext::CommandRecordingContext(CommandRecordingContext&& rhs)
        : mCommands(rhs.AcquireCommands()) {
    }

    CommandRecordingContext& CommandRecordingContext::operator=(CommandRecordingContext&& rhs) {
        mCommands = rhs.AcquireCommands();
        return *this;
    }

    CommandRecordingContext::~CommandRecordingContext() {
        // Commands must be acquired.
        ASSERT(mCommands == nullptr);
    }

    id<MTLCommandBuffer> CommandRecordingContext::GetCommands() {
        return mCommands.Get();
    }

    NSPRef<id<MTLCommandBuffer>> CommandRecordingContext::AcquireCommands() {
        // A blit encoder can be left open from WriteBuffer, make sure we close it.
        if (mCommands != nullptr) {
            EndBlit();
        }

        ASSERT(!mInEncoder);
        return std::move(mCommands);
    }

    id<MTLBlitCommandEncoder> CommandRecordingContext::EnsureBlit() {
        ASSERT(mCommands != nullptr);

        if (mBlit == nullptr) {
            ASSERT(!mInEncoder);
            mInEncoder = true;

            // The encoder is created autoreleased. Retain it to avoid the autoreleasepool from
            // draining from under us.
            mBlit = [*mCommands blitCommandEncoder];
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
        // The encoder is created autoreleased. Retain it to avoid the autoreleasepool from draining
        // from under us.
        mCompute = [*mCommands computeCommandEncoder];
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
        // The encoder is created autoreleased. Retain it to avoid the autoreleasepool from draining
        // from under us.
        mRender = [*mCommands renderCommandEncoderWithDescriptor:descriptor];
        return mRender.Get();
    }

    void CommandRecordingContext::EndRender() {
        ASSERT(mCommands != nullptr);
        ASSERT(mRender != nullptr);

        [*mRender endEncoding];
        mRender = nullptr;
        mInEncoder = false;
    }

}}  // namespace dawn_native::metal
