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

#ifndef BACKEND_COMMANDBUFFER_H_
#define BACKEND_COMMANDBUFFER_H_

#include "nxt/nxtcpp.h"

#include "backend/Builder.h"
#include "backend/CommandAllocator.h"
#include "backend/Error.h"
#include "backend/PassResourceUsage.h"
#include "backend/RefCounted.h"

#include <memory>
#include <set>
#include <utility>

namespace backend {

    class BindGroupBase;
    class BufferBase;
    class CommandBufferStateTracker;
    class FramebufferBase;
    class DeviceBase;
    class PipelineBase;
    class RenderPassBase;
    class TextureBase;

    class CommandBufferBuilder;

    class CommandBufferBase : public RefCounted {
      public:
        CommandBufferBase(CommandBufferBuilder* builder);

        DeviceBase* GetDevice();

      private:
        DeviceBase* mDevice;
    };

    class CommandBufferBuilder : public Builder<CommandBufferBase> {
      public:
        CommandBufferBuilder(DeviceBase* device);
        ~CommandBufferBuilder();

        MaybeError ValidateGetResult();

        CommandIterator AcquireCommands();
        std::vector<PassResourceUsage> AcquirePassResourceUsage();

        // NXT API
        void BeginComputePass();
        void BeginRenderPass(RenderPassDescriptorBase* info);
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
                                 uint32_t level);
        void CopyTextureToBuffer(TextureBase* texture,
                                 uint32_t x,
                                 uint32_t y,
                                 uint32_t z,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t depth,
                                 uint32_t level,
                                 BufferBase* buffer,
                                 uint32_t bufferOffset,
                                 uint32_t rowPitch);
        void Dispatch(uint32_t x, uint32_t y, uint32_t z);
        void DrawArrays(uint32_t vertexCount,
                        uint32_t instanceCount,
                        uint32_t firstVertex,
                        uint32_t firstInstance);
        void DrawElements(uint32_t vertexCount,
                          uint32_t instanceCount,
                          uint32_t firstIndex,
                          uint32_t firstInstance);
        void EndComputePass();
        void EndRenderPass();
        void SetPushConstants(dawn::ShaderStageBit stages,
                              uint32_t offset,
                              uint32_t count,
                              const void* data);
        void SetComputePipeline(ComputePipelineBase* pipeline);
        void SetRenderPipeline(RenderPipelineBase* pipeline);
        void SetStencilReference(uint32_t reference);
        void SetBlendColor(float r, float g, float b, float a);
        void SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void SetBindGroup(uint32_t groupIndex, BindGroupBase* group);
        void SetIndexBuffer(BufferBase* buffer, uint32_t offset);

        template <typename T>
        void SetVertexBuffers(uint32_t startSlot,
                              uint32_t count,
                              T* const* buffers,
                              uint32_t const* offsets) {
            static_assert(std::is_base_of<BufferBase, T>::value, "");
            SetVertexBuffers(startSlot, count, reinterpret_cast<BufferBase* const*>(buffers),
                             offsets);
        }
        void SetVertexBuffers(uint32_t startSlot,
                              uint32_t count,
                              BufferBase* const* buffers,
                              uint32_t const* offsets);

        void TransitionBufferUsage(BufferBase* buffer, dawn::BufferUsageBit usage);

      private:
        friend class CommandBufferBase;

        CommandBufferBase* GetResultImpl() override;
        void MoveToIterator();

        MaybeError ValidateComputePass();
        MaybeError ValidateRenderPass(RenderPassDescriptorBase* renderPass);

        std::unique_ptr<CommandBufferStateTracker> mState;
        CommandAllocator mAllocator;
        CommandIterator mIterator;
        bool mWasMovedToIterator = false;
        bool mWereCommandsAcquired = false;
        bool mWerePassUsagesAcquired = false;

        std::vector<PassResourceUsage> mPassResourceUsages;
    };

}  // namespace backend

#endif  // BACKEND_COMMANDBUFFER_H_
