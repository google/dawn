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

    CommandRecordingContext::CommandRecordingContext(id<MTLCommandBuffer> commands)
        : mCommands(commands) {
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
        ASSERT(mCommands == nil);
    }

    id<MTLCommandBuffer> CommandRecordingContext::GetCommands() {
        return mCommands;
    }

    id<MTLCommandBuffer> CommandRecordingContext::AcquireCommands() {
        ASSERT(!mInEncoder);

        id<MTLCommandBuffer> commands = mCommands;
        mCommands = nil;
        return commands;
    }

    id<MTLBlitCommandEncoder> CommandRecordingContext::EnsureBlit() {
        ASSERT(mCommands != nil);

        if (mBlit == nil) {
            ASSERT(!mInEncoder);
            mInEncoder = true;
            mBlit = [mCommands blitCommandEncoder];
        }
        return mBlit;
    }

    void CommandRecordingContext::EndBlit() {
        ASSERT(mCommands != nil);

        if (mBlit != nil) {
            [mBlit endEncoding];
            mBlit = nil;  // This will be autoreleased.
            mInEncoder = false;
        }
    }

    id<MTLComputeCommandEncoder> CommandRecordingContext::BeginCompute() {
        ASSERT(mCommands != nil);
        ASSERT(mCompute == nil);
        ASSERT(!mInEncoder);

        mInEncoder = true;
        mCompute = [mCommands computeCommandEncoder];
        return mCompute;
    }

    void CommandRecordingContext::EndCompute() {
        ASSERT(mCommands != nil);
        ASSERT(mCompute != nil);

        [mCompute endEncoding];
        mCompute = nil;  // This will be autoreleased.
        mInEncoder = false;
    }

    id<MTLRenderCommandEncoder> CommandRecordingContext::BeginRender(
        MTLRenderPassDescriptor* descriptor) {
        ASSERT(mCommands != nil);
        ASSERT(mRender == nil);
        ASSERT(!mInEncoder);

        mInEncoder = true;
        mRender = [mCommands renderCommandEncoderWithDescriptor:descriptor];
        return mRender;
    }

    void CommandRecordingContext::EndRender() {
        ASSERT(mCommands != nil);
        ASSERT(mRender != nil);

        [mRender endEncoding];
        mRender = nil;  // This will be autoreleased.
        mInEncoder = false;
    }

}}  // namespace dawn_native::metal
