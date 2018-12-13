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

#ifndef DAWNNATIVE_COMMANDS_H_
#define DAWNNATIVE_COMMANDS_H_

#include "dawn_native/RenderPassDescriptor.h"
#include "dawn_native/Texture.h"

#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    // Definition of the commands that are present in the CommandIterator given by the
    // CommandBufferBuilder. There are not defined in CommandBuffer.h to break some header
    // dependencies: Ref<Object> needs Object to be defined.

    enum class Command {
        BeginComputePass,
        BeginRenderPass,
        CopyBufferToBuffer,
        CopyBufferToTexture,
        CopyTextureToBuffer,
        Dispatch,
        Draw,
        DrawIndexed,
        EndComputePass,
        EndRenderPass,
        SetComputePipeline,
        SetRenderPipeline,
        SetPushConstants,
        SetStencilReference,
        SetScissorRect,
        SetBlendColor,
        SetBindGroup,
        SetIndexBuffer,
        SetVertexBuffers,
    };

    struct BeginComputePassCmd {};

    struct BeginRenderPassCmd {
        Ref<RenderPassDescriptorBase> info;
    };

    struct BufferCopy {
        Ref<BufferBase> buffer;
        uint32_t offset;       // Bytes
        uint32_t rowPitch;     // Bytes
        uint32_t imageHeight;  // Texels
    };

    struct TextureCopy {
        Ref<TextureBase> texture;
        uint32_t level;
        uint32_t slice;
        Origin3D origin;  // Texels
    };

    struct CopyBufferToBufferCmd {
        BufferCopy source;
        BufferCopy destination;
        uint32_t size;
    };

    struct CopyBufferToTextureCmd {
        BufferCopy source;
        TextureCopy destination;
        Extent3D copySize;  // Texels
    };

    struct CopyTextureToBufferCmd {
        TextureCopy source;
        BufferCopy destination;
        Extent3D copySize;  // Texels
    };

    struct DispatchCmd {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };

    struct DrawCmd {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };

    struct DrawIndexedCmd {
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t baseVertex;
        uint32_t firstInstance;
    };

    struct EndComputePassCmd {};

    struct EndRenderPassCmd {};

    struct SetComputePipelineCmd {
        Ref<ComputePipelineBase> pipeline;
    };

    struct SetRenderPipelineCmd {
        Ref<RenderPipelineBase> pipeline;
    };

    struct SetPushConstantsCmd {
        dawn::ShaderStageBit stages;
        uint32_t offset;
        uint32_t count;
    };

    struct SetStencilReferenceCmd {
        uint32_t reference;
    };

    struct SetScissorRectCmd {
        uint32_t x, y, width, height;
    };

    struct SetBlendColorCmd {
        float r, g, b, a;
    };

    struct SetBindGroupCmd {
        uint32_t index;
        Ref<BindGroupBase> group;
    };

    struct SetIndexBufferCmd {
        Ref<BufferBase> buffer;
        uint32_t offset;
    };

    struct SetVertexBuffersCmd {
        uint32_t startSlot;
        uint32_t count;
    };

    // This needs to be called before the CommandIterator is freed so that the Ref<> present in
    // the commands have a chance to run their destructor and remove internal references.
    class CommandIterator;
    void FreeCommands(CommandIterator* commands);

    // Helper function to allow skipping over a command when it is unimplemented, while still
    // consuming the correct amount of data from the command iterator.
    void SkipCommand(CommandIterator* commands, Command type);

}  // namespace dawn_native

#endif  // DAWNNATIVE_COMMANDS_H_
