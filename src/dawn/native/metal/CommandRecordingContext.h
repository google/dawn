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

// This class wraps a MTLCommandBuffer and tracks which Metal encoder is open.
// Only one encoder may be open at a time.
class CommandRecordingContext : NonMovable {
  public:
    CommandRecordingContext();
    ~CommandRecordingContext();

    id<MTLCommandBuffer> GetCommands();
    void MarkUsed();
    bool WasUsed() const;

    MaybeError PrepareNextCommandBuffer(id<MTLCommandQueue> queue);
    NSPRef<id<MTLCommandBuffer>> AcquireCommands();

    id<MTLBlitCommandEncoder> EnsureBlit();
    void EndBlit();

    id<MTLComputeCommandEncoder> BeginCompute();
    void EndCompute();

    id<MTLRenderCommandEncoder> BeginRender(MTLRenderPassDescriptor* descriptor);
    void EndRender();

  private:
    NSPRef<id<MTLCommandBuffer>> mCommands;
    NSPRef<id<MTLBlitCommandEncoder>> mBlit;
    NSPRef<id<MTLComputeCommandEncoder>> mCompute;
    NSPRef<id<MTLRenderCommandEncoder>> mRender;
    bool mInEncoder = false;
    bool mUsed = false;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_COMMANDRECORDINGCONTEXT_H_
