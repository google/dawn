// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef SRC_DAWN_NATIVE_METAL_COMMANDRECORDINGCONTEXT_H_
#define SRC_DAWN_NATIVE_METAL_COMMANDRECORDINGCONTEXT_H_

#include "dawn/common/NSRef.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/native/Error.h"

#import <Metal/Metal.h>

namespace dawn::native::metal {

struct MTLSharedEventAndSignalValue {
    NSPRef<id> sharedEvent;
    uint64_t signaledValue;
};

// This class wraps a MTLCommandBuffer and tracks which Metal encoder is open.
// Only one encoder may be open at a time.
class CommandRecordingContext : NonMovable {
  public:
    CommandRecordingContext();
    ~CommandRecordingContext();

    id<MTLCommandBuffer> GetCommands();
    void SetNeedsSubmit();
    bool NeedsSubmit() const;
    void MarkUsed();
    bool WasUsed() const;

    MaybeError PrepareNextCommandBuffer(id<MTLCommandQueue> queue);
    NSPRef<id<MTLCommandBuffer>> AcquireCommands();

    // Create blit pass encoder from blit pass descriptor
    id<MTLBlitCommandEncoder> BeginBlit(MTLBlitPassDescriptor* descriptor)
        API_AVAILABLE(macos(11.0), ios(14.0));
    id<MTLBlitCommandEncoder> EnsureBlit();
    void EndBlit();

    // Create a sequential compute pass by default.
    id<MTLComputeCommandEncoder> BeginCompute();
    // Create configurable compute pass from a descriptor with serial dispatch type which commands
    // are executed sequentially.
    id<MTLComputeCommandEncoder> BeginCompute(MTLComputePassDescriptor* descriptor)
        API_AVAILABLE(macos(11.0), ios(14.0));
    void EndCompute();

    id<MTLRenderCommandEncoder> BeginRender(MTLRenderPassDescriptor* descriptor);
    void EndRender();

  private:
    NSPRef<id<MTLCommandBuffer>> mCommands;
    NSPRef<id<MTLBlitCommandEncoder>> mBlit;
    NSPRef<id<MTLComputeCommandEncoder>> mCompute;
    NSPRef<id<MTLRenderCommandEncoder>> mRender;
    bool mInEncoder = false;
    bool mNeedsSubmit = false;
    bool mUsed = false;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_COMMANDRECORDINGCONTEXT_H_
