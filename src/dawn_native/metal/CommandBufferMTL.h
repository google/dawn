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

#ifndef DAWNNATIVE_METAL_COMMANDBUFFERMTL_H_
#define DAWNNATIVE_METAL_COMMANDBUFFERMTL_H_

#include "dawn_native/CommandBuffer.h"

#import <Metal/Metal.h>

namespace backend {
    class RenderPassDescriptorBase;
}

namespace backend { namespace metal {

    class Device;

    class CommandBuffer : public CommandBufferBase {
      public:
        CommandBuffer(CommandBufferBuilder* builder);
        ~CommandBuffer();

        void FillCommands(id<MTLCommandBuffer> commandBuffer);

      private:
        void EncodeComputePass(id<MTLCommandBuffer> commandBuffer);
        void EncodeRenderPass(id<MTLCommandBuffer> commandBuffer,
                              RenderPassDescriptorBase* renderPass);

        Device* mDevice;
        CommandIterator mCommands;
    };

}}  // namespace backend::metal

#endif  // DAWNNATIVE_METAL_COMMANDBUFFERMTL_H_
