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

#ifndef SRC_DAWN_NATIVE_COMMANDS_H_
#define SRC_DAWN_NATIVE_COMMANDS_H_

#include <array>
#include <bitset>
#include <vector>

#include "dawn/common/Constants.h"

#include "dawn/native/AttachmentState.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/Texture.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

// Definition of the commands that are present in the CommandIterator given by the
// CommandBufferBuilder. There are not defined in CommandBuffer.h to break some header
// dependencies: Ref<Object> needs Object to be defined.

enum class Command {
    BeginComputePass,
    BeginOcclusionQuery,
    BeginRenderPass,
    ClearBuffer,
    CopyBufferToBuffer,
    CopyBufferToTexture,
    CopyTextureToBuffer,
    CopyTextureToTexture,
    Dispatch,
    DispatchIndirect,
    Draw,
    DrawIndexed,
    DrawIndirect,
    DrawIndexedIndirect,
    EndComputePass,
    EndOcclusionQuery,
    EndRenderPass,
    ExecuteBundles,
    InsertDebugMarker,
    PopDebugGroup,
    PushDebugGroup,
    ResolveQuerySet,
    SetComputePipeline,
    SetRenderPipeline,
    SetStencilReference,
    SetViewport,
    SetScissorRect,
    SetBlendConstant,
    SetBindGroup,
    SetIndexBuffer,
    SetVertexBuffer,
    WriteBuffer,
    WriteTimestamp,
};

struct TimestampWrite {
    Ref<QuerySetBase> querySet;
    uint32_t queryIndex;
};

struct BeginComputePassCmd {
    std::vector<TimestampWrite> timestampWrites;
};

struct BeginOcclusionQueryCmd {
    Ref<QuerySetBase> querySet;
    uint32_t queryIndex;
};

struct RenderPassColorAttachmentInfo {
    Ref<TextureViewBase> view;
    Ref<TextureViewBase> resolveTarget;
    wgpu::LoadOp loadOp;
    wgpu::StoreOp storeOp;
    dawn::native::Color clearColor;
};

struct RenderPassDepthStencilAttachmentInfo {
    Ref<TextureViewBase> view;
    wgpu::LoadOp depthLoadOp;
    wgpu::StoreOp depthStoreOp;
    wgpu::LoadOp stencilLoadOp;
    wgpu::StoreOp stencilStoreOp;
    float clearDepth;
    uint32_t clearStencil;
    bool depthReadOnly;
    bool stencilReadOnly;
};

struct BeginRenderPassCmd {
    Ref<AttachmentState> attachmentState;
    ityp::array<ColorAttachmentIndex, RenderPassColorAttachmentInfo, kMaxColorAttachments>
        colorAttachments;
    RenderPassDepthStencilAttachmentInfo depthStencilAttachment;

    // Cache the width and height of all attachments for convenience
    uint32_t width;
    uint32_t height;

    Ref<QuerySetBase> occlusionQuerySet;
    std::vector<TimestampWrite> timestampWrites;
};

struct BufferCopy {
    Ref<BufferBase> buffer;
    uint64_t offset;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
};

struct TextureCopy {
    Ref<TextureBase> texture;
    uint32_t mipLevel;
    Origin3D origin;  // Texels / array layer
    Aspect aspect;
};

struct CopyBufferToBufferCmd {
    Ref<BufferBase> source;
    uint64_t sourceOffset;
    Ref<BufferBase> destination;
    uint64_t destinationOffset;
    uint64_t size;
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

struct CopyTextureToTextureCmd {
    TextureCopy source;
    TextureCopy destination;
    Extent3D copySize;  // Texels
};

struct DispatchCmd {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct DispatchIndirectCmd {
    Ref<BufferBase> indirectBuffer;
    uint64_t indirectOffset;
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
    int32_t baseVertex;
    uint32_t firstInstance;
};

struct DrawIndirectCmd {
    Ref<BufferBase> indirectBuffer;
    uint64_t indirectOffset;
};

struct DrawIndexedIndirectCmd : DrawIndirectCmd {};

struct EndComputePassCmd {
    std::vector<TimestampWrite> timestampWrites;
};

struct EndOcclusionQueryCmd {
    Ref<QuerySetBase> querySet;
    uint32_t queryIndex;
};

struct EndRenderPassCmd {
    std::vector<TimestampWrite> timestampWrites;
};

struct ExecuteBundlesCmd {
    uint32_t count;
};

struct ClearBufferCmd {
    Ref<BufferBase> buffer;
    uint64_t offset;
    uint64_t size;
};

struct InsertDebugMarkerCmd {
    uint32_t length;
};

struct PopDebugGroupCmd {};

struct PushDebugGroupCmd {
    uint32_t length;
};

struct ResolveQuerySetCmd {
    Ref<QuerySetBase> querySet;
    uint32_t firstQuery;
    uint32_t queryCount;
    Ref<BufferBase> destination;
    uint64_t destinationOffset;
};

struct SetComputePipelineCmd {
    Ref<ComputePipelineBase> pipeline;
};

struct SetRenderPipelineCmd {
    Ref<RenderPipelineBase> pipeline;
};

struct SetStencilReferenceCmd {
    uint32_t reference;
};

struct SetViewportCmd {
    float x, y, width, height, minDepth, maxDepth;
};

struct SetScissorRectCmd {
    uint32_t x, y, width, height;
};

struct SetBlendConstantCmd {
    Color color;
};

struct SetBindGroupCmd {
    BindGroupIndex index;
    Ref<BindGroupBase> group;
    uint32_t dynamicOffsetCount;
};

struct SetIndexBufferCmd {
    Ref<BufferBase> buffer;
    wgpu::IndexFormat format;
    uint64_t offset;
    uint64_t size;
};

struct SetVertexBufferCmd {
    VertexBufferSlot slot;
    Ref<BufferBase> buffer;
    uint64_t offset;
    uint64_t size;
};

struct WriteBufferCmd {
    Ref<BufferBase> buffer;
    uint64_t offset;
    uint64_t size;
};

struct WriteTimestampCmd {
    Ref<QuerySetBase> querySet;
    uint32_t queryIndex;
};

// This needs to be called before the CommandIterator is freed so that the Ref<> present in
// the commands have a chance to run their destructor and remove internal references.
class CommandIterator;
void FreeCommands(CommandIterator* commands);

// Helper function to allow skipping over a command when it is unimplemented, while still
// consuming the correct amount of data from the command iterator.
void SkipCommand(CommandIterator* commands, Command type);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDS_H_
