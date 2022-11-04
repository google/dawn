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
