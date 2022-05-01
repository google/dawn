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

#ifndef SRC_DAWN_NATIVE_OPENGL_COMMANDBUFFERGL_H_
#define SRC_DAWN_NATIVE_OPENGL_COMMANDBUFFERGL_H_

#include "dawn/native/CommandBuffer.h"

namespace dawn::native {
struct BeginRenderPassCmd;
}  // namespace dawn::native

namespace dawn::native::opengl {

class Device;
struct OpenGLFunctions;

class CommandBuffer final : public CommandBufferBase {
  public:
    CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor);

    MaybeError Execute();

  private:
    MaybeError ExecuteComputePass();
    MaybeError ExecuteRenderPass(BeginRenderPassCmd* renderPass);
};

// Like glTexSubImage*, the "data" argument is either a pointer to image data or
// an offset if a PBO is bound.
void DoTexSubImage(const OpenGLFunctions& gl,
                   const TextureCopy& destination,
                   const void* data,
                   const TextureDataLayout& dataLayout,
                   const Extent3D& copySize);
}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_COMMANDBUFFERGL_H_
