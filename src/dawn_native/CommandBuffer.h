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

#ifndef DAWNNATIVE_COMMANDBUFFER_H_
#define DAWNNATIVE_COMMANDBUFFER_H_

#include "dawn_native/dawn_platform.h"

#include "dawn_native/Builder.h"
#include "dawn_native/CommandAllocator.h"
#include "dawn_native/Error.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/PassResourceUsage.h"

#include <memory>
#include <set>
#include <utility>

namespace dawn_native {

    class BindGroupBase;
    class BufferBase;
    class FramebufferBase;
    class DeviceBase;
    class PipelineBase;
    class RenderPassBase;
    class TextureBase;

    class CommandBufferBuilder;

    class CommandBufferBase : public ObjectBase {
      public:
        CommandBufferBase(CommandBufferBuilder* builder);
    };

    class CommandBufferBuilder : public Builder<CommandBufferBase> {
      public:
        CommandBufferBuilder(DeviceBase* device);
        ~CommandBufferBuilder();

        MaybeError ValidateGetResult();

        CommandIterator AcquireCommands();
        std::vector<PassResourceUsage> AcquirePassResourceUsage();

        // Dawn API
        ComputePassEncoderBase* BeginComputePass();
        RenderPassEncoderBase* BeginRenderPass(RenderPassDescriptorBase* info);
        void CopyBufferToBuffer(BufferBase* source,
                                uint32_t sourceOffset,
                                BufferBase* destination,
                                uint32_t destinationOffset,
                                uint32_t size);
        void CopyBufferToTexture(BufferBase* buffer,
                                 uint32_t bufferOffset,
                                 uint32_t rowPitch,
                                 TextureBase* texture,
                                 uint32_t x,
                                 uint32_t y,
                                 uint32_t z,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t depth,
                                 uint32_t level,
                                 uint32_t slice);
        void CopyTextureToBuffer(TextureBase* texture,
                                 uint32_t x,
                                 uint32_t y,
                                 uint32_t z,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t depth,
                                 uint32_t level,
                                 uint32_t slice,
                                 BufferBase* buffer,
                                 uint32_t bufferOffset,
                                 uint32_t rowPitch);

        // Functions to interact with the encoders
        bool ConsumedError(MaybeError maybeError) {
            if (DAWN_UNLIKELY(maybeError.IsError())) {
                ConsumeError(maybeError.AcquireError());
                return true;
            }
            return false;
        }

        void PassEnded();

      private:
        friend class CommandBufferBase;

        enum class EncodingState : uint8_t;
        EncodingState mEncodingState;

        CommandBufferBase* GetResultImpl() override;
        void MoveToIterator();

        MaybeError ValidateComputePass();
        MaybeError ValidateRenderPass(RenderPassDescriptorBase* renderPass);

        void ConsumeError(ErrorData* error);

        CommandAllocator mAllocator;
        CommandIterator mIterator;
        bool mWasMovedToIterator = false;
        bool mWereCommandsAcquired = false;
        bool mWerePassUsagesAcquired = false;

        std::vector<PassResourceUsage> mPassResourceUsages;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_COMMANDBUFFER_H_
