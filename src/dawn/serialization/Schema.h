// Copyright 2025 The Dawn & Tint Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS-IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_DAWN_SERIALIZATION_SCHEMA_H_
#define SRC_DAWN_SERIALIZATION_SCHEMA_H_

#include <cstdint>
#include <string>
#include <vector>

namespace schema {
// NOTE: This file must be included after files that define
// DAWN_REPLAY_SERIALIZABLE and DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA.
// Those macro are different for serialization and deserialization.

// TODO(crbug.com/413053623): Switch to protobufs or json. This is just to get
// stuff working. We'll switch to something else once we're sure of the data we
// want to capture.

using ObjectId = uint64_t;

enum class ObjectType : uint32_t {
    Invalid = 0,  // 0 is invalid at it's more likely to catch bugs.
    BindGroup,
    BindGroupLayout,
    Buffer,
    CommandBuffer,
    ComputePipeline,
    Device,
    PipelineLayout,
    QuerySet,
    RenderPipeline,
    Sampler,
    ShaderModule,
    Texture,
    TextureView,
};

enum class BindGroupLayoutEntryType : uint32_t {
    Invalid = 0,  // 0 is invalid at it's more likely to catch bugs.
    BufferBinding,
    SamplerBinding,
    TextureBinding,
    TexelBufferBinding,
    StorageTextureBinding,
    ExternalTextureBinding,
    StaticSamplerBindingInfo,
    InputAttachmentBindingInfo,
};

enum class ComputePassCommand : uint32_t {
    Invalid = 0,  // 0 is invalid at it's more likely to catch bugs.
    Dispatch,
    DispatchIndirect,
    SetComputePipeline,
    SetBindGroup,
    WriteTimestamp,
    InsertDebugMarker,
    PopDebugGroup,
    PushDebugGroup,
    SetImmediateData,
    End,
};

enum class RenderPassCommand : uint32_t {
    Invalid = 0,  // 0 is invalid at it's more likely to catch bugs.
    Draw,
    DrawIndexed,
    DrawIndirect,
    DrawIndexedIndirect,
    SetPipeline,
    SetBindGroup,
    SetIndexBuffer,
    SetVertexBuffer,
    SetViewport,
    SetScissorRect,
    SetBlendConstant,
    SetStencilReference,
    EndRenderPass,
    ExecuteBundles,
    WriteTimestamp,
    InsertDebugMarker,
    PopDebugGroup,
    PushDebugGroup,
    SetImmediateData,
    End,
};

enum class EncoderCommand : uint32_t {
    Invalid = 0,  // 0 is invalid at it's more likely to catch bugs.
    BeginComputePass,
    BeginRenderPass,
    CopyBufferToBuffer,
    CopyBufferToTexture,
    CopyTextureToBuffer,
    CopyTextureToTexture,
    ClearBuffer,
    ResolveQuerySet,
    WriteTimestamp,
    InsertDebugMarker,
    PopDebugGroup,
    PushDebugGroup,
    WriteBuffer,
    End,
};

enum class RootCommand : uint32_t {
    Invalid = 0,  // 0 is invalid at it's more likely to catch bugs.
    CreateResource,
    QueueSubmit,
    WriteBuffer,
    WriteTexture,
    UnmapBuffer,

    End,
};

#define ORIGIN3D_MEMBER(X) \
    X(uint32_t, x)         \
    X(uint32_t, y)         \
    X(uint32_t, z)

DAWN_REPLAY_SERIALIZABLE(struct, Origin3D, ORIGIN3D_MEMBER){};

#define EXTENT3D_MEMBER(X) \
    X(uint32_t, width)     \
    X(uint32_t, height)    \
    X(uint32_t, depthOrArrayLayers)

DAWN_REPLAY_SERIALIZABLE(struct, Extent3D, EXTENT3D_MEMBER){};

#define COLOR_MEMBER(X) \
    X(double, r)        \
    X(double, g)        \
    X(double, b)        \
    X(double, a)

DAWN_REPLAY_SERIALIZABLE(struct, Color, COLOR_MEMBER){};

#define PIPELINE_CONSTANT_MEMBER(X) \
    X(std::string, name)            \
    X(double, value)

DAWN_REPLAY_SERIALIZABLE(struct, PipelineConstant, PIPELINE_CONSTANT_MEMBER){};

#define PROGRAMMABLE_STAGE_MEMBER(X) \
    X(ObjectId, moduleId)            \
    X(std::string, entryPoint)       \
    X(std::vector<PipelineConstant>, constants)

DAWN_REPLAY_SERIALIZABLE(struct, ProgrammableStage, PROGRAMMABLE_STAGE_MEMBER){};

#define VERTEX_ATTRIBUTE_MEMBER(X) \
    X(wgpu::VertexFormat, format)  \
    X(uint64_t, offset)            \
    X(uint32_t, shaderLocation)

DAWN_REPLAY_SERIALIZABLE(struct, VertexAttribute, VERTEX_ATTRIBUTE_MEMBER){};

#define VERTEX_BUFFER_LAYOUT_MEMBER(X) \
    X(uint64_t, arrayStride)           \
    X(wgpu::VertexStepMode, stepMode)  \
    X(std::vector<VertexAttribute>, attributes)

DAWN_REPLAY_SERIALIZABLE(struct, VertexBufferLayout, VERTEX_BUFFER_LAYOUT_MEMBER){};

#define VERTEX_STATE_MEMBER(X)    \
    X(ProgrammableStage, program) \
    X(std::vector<VertexBufferLayout>, buffers)

DAWN_REPLAY_SERIALIZABLE(struct, VertexState, VERTEX_STATE_MEMBER){};

#define PRIMITIVE_STATE_MEMBER(X)          \
    X(wgpu::PrimitiveTopology, topology)   \
    X(wgpu::IndexFormat, stripIndexFormat) \
    X(wgpu::FrontFace, frontFace)          \
    X(wgpu::CullMode, cullMode)            \
    X(bool, unclippedDepth)

DAWN_REPLAY_SERIALIZABLE(struct, PrimitiveState, PRIMITIVE_STATE_MEMBER){};

#define STENCIL_FACE_STATE_MEMBER(X)       \
    X(wgpu::CompareFunction, compare)      \
    X(wgpu::StencilOperation, failOp)      \
    X(wgpu::StencilOperation, depthFailOp) \
    X(wgpu::StencilOperation, passOp)

DAWN_REPLAY_SERIALIZABLE(struct, StencilFaceState, STENCIL_FACE_STATE_MEMBER){};

#define DEPTH_STENCIL_STATE_MEMBER(X)      \
    X(wgpu::TextureFormat, format)         \
    X(bool, depthWriteEnabled)             \
    X(wgpu::CompareFunction, depthCompare) \
    X(StencilFaceState, stencilFront)      \
    X(StencilFaceState, stencilBack)       \
    X(uint32_t, stencilReadMask)           \
    X(uint32_t, stencilWriteMask)          \
    X(int32_t, depthBias)                  \
    X(float, depthBiasSlopeScale)          \
    X(float, depthBiasClamp)

DAWN_REPLAY_SERIALIZABLE(struct, DepthStencilState, DEPTH_STENCIL_STATE_MEMBER){};

#define MULTISAMPLE_STATE_MEMBER(X) \
    X(uint32_t, count)              \
    X(uint32_t, mask)               \
    X(bool, alphaToCoverageEnabled)

DAWN_REPLAY_SERIALIZABLE(struct, MultisampleState, MULTISAMPLE_STATE_MEMBER){};

#define BLEND_COMPONENT_MEMBER(X)      \
    X(wgpu::BlendOperation, operation) \
    X(wgpu::BlendFactor, srcFactor)    \
    X(wgpu::BlendFactor, dstFactor)

DAWN_REPLAY_SERIALIZABLE(struct, BlendComponent, BLEND_COMPONENT_MEMBER){};

#define BLEND_STATE_MEMBER(X) \
    X(BlendComponent, color)  \
    X(BlendComponent, alpha)

DAWN_REPLAY_SERIALIZABLE(struct, BlendState, BLEND_STATE_MEMBER){};

#define COLOR_TARGET_STATE_MEMBER(X) \
    X(wgpu::TextureFormat, format)   \
    X(BlendState, blend)             \
    X(wgpu::ColorWriteMask, writeMask)

DAWN_REPLAY_SERIALIZABLE(struct, ColorTargetState, COLOR_TARGET_STATE_MEMBER){};

#define FRAGMENT_STATE_MEMBER(X)  \
    X(ProgrammableStage, program) \
    X(std::vector<ColorTargetState>, targets)

DAWN_REPLAY_SERIALIZABLE(struct, FragmentState, FRAGMENT_STATE_MEMBER){};

#define BIND_GROUP_LAYOUT_BINDING_MEMBER(X) \
    X(uint32_t, binding)                    \
    X(wgpu::ShaderStage, visibility)        \
    X(uint32_t, bindingArraySize)

DAWN_REPLAY_SERIALIZABLE(struct, BindGroupLayoutBinding, BIND_GROUP_LAYOUT_BINDING_MEMBER){};

#define BUFFER_BIND_GROUP_LAYOUT_MEMBER(X) \
    X(wgpu::BufferBindingType, type)       \
    X(uint64_t, minBindingSize)            \
    X(bool, hasDynamicOffset)

DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(BufferBinding, BUFFER_BIND_GROUP_LAYOUT_MEMBER){};

#define STORAGE_TEXTURE_BIND_GROUP_LAYOUT_MEMBER(X) \
    X(wgpu::TextureFormat, format)                  \
    X(wgpu::TextureViewDimension, viewDimension)    \
    X(wgpu::StorageTextureAccess, access)

DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(StorageTextureBinding,
                                          STORAGE_TEXTURE_BIND_GROUP_LAYOUT_MEMBER){};

#define TEXTURE_BIND_GROUP_LAYOUT_MEMBER(X)      \
    X(wgpu::TextureSampleType, sampleType)       \
    X(wgpu::TextureViewDimension, viewDimension) \
    X(bool, multisampled)

DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(TextureBinding, TEXTURE_BIND_GROUP_LAYOUT_MEMBER){};

#define BIND_GROUP_LAYOUT_MEMBER(X) X(uint32_t, numEntries)

DAWN_REPLAY_SERIALIZABLE(struct, BindGroupLayout, BIND_GROUP_LAYOUT_MEMBER){};

#define PIPELINE_LAYOUT_MEMBER(X) X(std::vector<ObjectId>, bindGroupLayoutIds)

DAWN_REPLAY_SERIALIZABLE(struct, PipelineLayout, PIPELINE_LAYOUT_MEMBER){};

#define BUFFER_CREATION_MEMBER(X) \
    X(uint64_t, size)             \
    X(wgpu::BufferUsage, usage)

DAWN_REPLAY_SERIALIZABLE(struct, Buffer, BUFFER_CREATION_MEMBER){};

#define TEXTURE_CREATION_MEMBER(X)       \
    X(wgpu::TextureUsage, usage)         \
    X(wgpu::TextureDimension, dimension) \
    X(Extent3D, size)                    \
    X(wgpu::TextureFormat, format)       \
    X(uint32_t, mipLevelCount)           \
    X(uint32_t, sampleCount)             \
    X(std::vector<wgpu::TextureFormat>, viewFormats)

DAWN_REPLAY_SERIALIZABLE(struct, Texture, TEXTURE_CREATION_MEMBER){};

#define TEXTURE_VIEW_CREATION_MEMBER(X)      \
    X(ObjectId, textureId)                   \
    X(wgpu::TextureFormat, format)           \
    X(wgpu::TextureViewDimension, dimension) \
    X(uint32_t, baseMipLevel)                \
    X(uint32_t, mipLevelCount)               \
    X(uint32_t, baseArrayLayer)              \
    X(uint32_t, arrayLayerCount)             \
    X(wgpu::TextureAspect, aspect)           \
    X(wgpu::TextureUsage, usage)

DAWN_REPLAY_SERIALIZABLE(struct, TextureView, TEXTURE_VIEW_CREATION_MEMBER){};

#define QUERYSET_CREATION_MEMBER(X) \
    X(wgpu::QueryType, type)        \
    X(uint32_t, count)

DAWN_REPLAY_SERIALIZABLE(struct, QuerySet, QUERYSET_CREATION_MEMBER){};

// TODO(452840621): Make this use a chain instead of hard coded to WGSL only and handle other
// chained structs.
#define SHADER_MODULE_CREATION_MEMBER(X) X(std::string, code)

DAWN_REPLAY_SERIALIZABLE(struct, ShaderModule, SHADER_MODULE_CREATION_MEMBER){};

#define BIND_GROUP_LAYOUT_INDEX_ID_PAIR(X) \
    X(uint32_t, groupIndex)                \
    X(ObjectId, bindGroupLayoutId)

DAWN_REPLAY_SERIALIZABLE(struct, BindGroupLayoutIndexIdPair, BIND_GROUP_LAYOUT_INDEX_ID_PAIR){};

#define COMPUTE_PIPELINE_CREATION_MEMBER(X) \
    X(ObjectId, layoutId)                   \
    X(ProgrammableStage, compute)           \
    X(std::vector<BindGroupLayoutIndexIdPair>, groupIndexIds)

DAWN_REPLAY_SERIALIZABLE(struct, ComputePipeline, COMPUTE_PIPELINE_CREATION_MEMBER){};

#define RENDER_PIPELINE_CREATION_MEMBER(X) \
    X(ObjectId, layoutId)                  \
    X(VertexState, vertex)                 \
    X(PrimitiveState, primitive)           \
    X(DepthStencilState, depthStencil)     \
    X(MultisampleState, multisample)       \
    X(FragmentState, fragment)             \
    X(std::vector<BindGroupLayoutIndexIdPair>, groupIndexIds)

DAWN_REPLAY_SERIALIZABLE(struct, RenderPipeline, RENDER_PIPELINE_CREATION_MEMBER){};

#define BUFFER_BIND_GROUP_ENTRY_MEMBER(X) \
    X(ObjectId, bufferId)                 \
    X(uint64_t, offset)                   \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(BufferBinding, BUFFER_BIND_GROUP_ENTRY_MEMBER){};

#define TEXTURE_BIND_GROUP_ENTRY_MEMBER(X) X(ObjectId, textureViewId)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(TextureBinding, TEXTURE_BIND_GROUP_ENTRY_MEMBER){};

#define BIND_GROUP_CREATION_MEMBER(X) \
    X(ObjectId, layoutId)             \
    X(uint32_t, numEntries)

DAWN_REPLAY_SERIALIZABLE(struct, BindGroup, BIND_GROUP_CREATION_MEMBER){};

#define LABELED_RESOURCE_MEMBER(X) \
    X(ObjectType, type)            \
    X(ObjectId, id)                \
    X(std::string, label)

DAWN_REPLAY_SERIALIZABLE(struct, LabeledResource, LABELED_RESOURCE_MEMBER){};

#define TEXEL_COPY_BUFFER_LAYOUT_MEMBER(X) \
    X(uint64_t, offset)                    \
    X(uint32_t, bytesPerRow)               \
    X(uint32_t, rowsPerImage)

DAWN_REPLAY_SERIALIZABLE(struct, TexelCopyBufferLayout, TEXEL_COPY_BUFFER_LAYOUT_MEMBER){};

#define TEXEL_COPY_BUFFER_INFO_MEMBER(X) \
    X(ObjectId, bufferId)                \
    X(TexelCopyBufferLayout, layout)

DAWN_REPLAY_SERIALIZABLE(struct, TexelCopyBufferInfo, TEXEL_COPY_BUFFER_INFO_MEMBER){};

#define TEXEL_COPY_TEXTURE_INFO_MEMBER(X) \
    X(ObjectId, textureId)                \
    X(uint32_t, mipLevel)                 \
    X(Origin3D, origin)                   \
    X(wgpu::TextureAspect, aspect)

DAWN_REPLAY_SERIALIZABLE(struct, TexelCopyTextureInfo, TEXEL_COPY_TEXTURE_INFO_MEMBER){};

#define TIMESTAMP_WRITES_MEMBER(X)         \
    X(ObjectId, querySetId)                \
    X(uint32_t, beginningOfPassWriteIndex) \
    X(uint32_t, endOfPassWriteIndex)

DAWN_REPLAY_SERIALIZABLE(struct, TimestampWrites, TIMESTAMP_WRITES_MEMBER){};

#define COLOR_ATTACHMENT_MEMBER(X) \
    X(ObjectId, viewId)            \
    X(uint32_t, depthSlice)        \
    X(ObjectId, resolveTargetId)   \
    X(wgpu::LoadOp, loadOp)        \
    X(wgpu::StoreOp, storeOp)      \
    X(Color, clearValue)

DAWN_REPLAY_SERIALIZABLE(struct, ColorAttachment, COLOR_ATTACHMENT_MEMBER){};

#define RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_MEMBER(X) \
    X(ObjectId, viewId)                                \
    X(wgpu::LoadOp, depthLoadOp)                       \
    X(wgpu::StoreOp, depthStoreOp)                     \
    X(float, depthClearValue)                          \
    X(bool, depthReadOnly)                             \
    X(wgpu::LoadOp, stencilLoadOp)                     \
    X(wgpu::StoreOp, stencilStoreOp)                   \
    X(uint32_t, stencilClearValue)                     \
    X(bool, stencilReadOnly)

DAWN_REPLAY_SERIALIZABLE(struct,
                         RenderPassDepthStencilAttachment,
                         RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_MEMBER){};

#define CREATE_RESOURCE_CMD_DATA_MEMBER(X) X(LabeledResource, resource)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(CreateResource, CREATE_RESOURCE_CMD_DATA_MEMBER){};

#define WRITE_BUFFER_CMD_DATA_MEMBER(X) \
    X(ObjectId, bufferId)               \
    X(uint64_t, bufferOffset)           \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(WriteBuffer, WRITE_BUFFER_CMD_DATA_MEMBER){};

#define UNMAP_BUFFER_CMD_DATA_MEMBER(X) \
    X(ObjectId, bufferId)               \
    X(uint64_t, bufferOffset)           \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(UnmapBuffer, UNMAP_BUFFER_CMD_DATA_MEMBER){};

#define WRITE_TEXTURE_CMD_DATA_MEMBER(X) \
    X(TexelCopyTextureInfo, destination) \
    X(TexelCopyBufferLayout, layout)     \
    X(Extent3D, size)                    \
    X(uint64_t, dataSize)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(WriteTexture, WRITE_TEXTURE_CMD_DATA_MEMBER){};

#define QUEUE_SUBMIT_CMD_DATA_MEMBER(X) X(std::vector<ObjectId>, commandBuffers)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(QueueSubmit, QUEUE_SUBMIT_CMD_DATA_MEMBER){};

#define COPY_BUFFER_TO_BUFFER_CMD_DATA_MEMBER(X) \
    X(ObjectId, srcBufferId)                     \
    X(uint64_t, srcOffset)                       \
    X(ObjectId, dstBufferId)                     \
    X(uint64_t, dstOffset)                       \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_ENCODER_CMD_AND_CMD_DATA(CopyBufferToBuffer,
                                          COPY_BUFFER_TO_BUFFER_CMD_DATA_MEMBER){};

#define COPY_BUFFER_TO_TEXTURE_CMD_DATA_MEMBER(X) \
    X(TexelCopyBufferInfo, source)                \
    X(TexelCopyTextureInfo, destination)          \
    X(Extent3D, copySize)

DAWN_REPLAY_MAKE_ENCODER_CMD_AND_CMD_DATA(CopyBufferToTexture,
                                          COPY_BUFFER_TO_TEXTURE_CMD_DATA_MEMBER){};

#define COPY_TEXTURE_TO_BUFFER_CMD_DATA_MEMBER(X) \
    X(TexelCopyTextureInfo, source)               \
    X(TexelCopyBufferInfo, destination)           \
    X(Extent3D, copySize)

DAWN_REPLAY_MAKE_ENCODER_CMD_AND_CMD_DATA(CopyTextureToBuffer,
                                          COPY_TEXTURE_TO_BUFFER_CMD_DATA_MEMBER){};

#define COPY_TEXTURE_TO_TEXTURE_CMD_DATA_MEMBER(X) \
    X(TexelCopyTextureInfo, source)                \
    X(TexelCopyTextureInfo, destination)           \
    X(Extent3D, copySize)

DAWN_REPLAY_MAKE_ENCODER_CMD_AND_CMD_DATA(CopyTextureToTexture,
                                          COPY_TEXTURE_TO_TEXTURE_CMD_DATA_MEMBER){};

#define BEGIN_COMPUTE_PASS_CMD_DATA_MEMBER(X) \
    X(std::string, label)                     \
    X(TimestampWrites, timestampWrites)

DAWN_REPLAY_MAKE_ENCODER_CMD_AND_CMD_DATA(BeginComputePass, BEGIN_COMPUTE_PASS_CMD_DATA_MEMBER){};

#define BEGIN_RENDER_PASS_CMD_DATA_MEMBER(X)                    \
    X(std::string, label)                                       \
    X(std::vector<ColorAttachment>, colorAttachments)           \
    X(RenderPassDepthStencilAttachment, depthStencilAttachment) \
    X(ObjectId, occlusionQuerySetId)                            \
    X(TimestampWrites, timestampWrites)                         \
    X(uint64_t, maxDrawCount)

DAWN_REPLAY_MAKE_ENCODER_CMD_AND_CMD_DATA(BeginRenderPass, BEGIN_RENDER_PASS_CMD_DATA_MEMBER){};

#define SET_COMPUTE_PIPELINE_CMD_DATA_MEMBER(X) X(ObjectId, pipelineId)

DAWN_REPLAY_MAKE_COMPUTE_PASS_CMD_AND_CMD_DATA(SetComputePipeline,
                                               SET_COMPUTE_PIPELINE_CMD_DATA_MEMBER){};

#define SET_BIND_GROUP_CMD_DATA_MEMBER(X) \
    X(uint32_t, index)                    \
    X(ObjectId, bindGroupId)              \
    X(std::vector<uint32_t>, dynamicOffsets)

DAWN_REPLAY_MAKE_COMPUTE_PASS_CMD_AND_CMD_DATA(SetBindGroup, SET_BIND_GROUP_CMD_DATA_MEMBER){};

#define DISPATCH_CMD_DATA_MEMBER(X) \
    X(uint32_t, x)                  \
    X(uint32_t, y)                  \
    X(uint32_t, z)

DAWN_REPLAY_MAKE_COMPUTE_PASS_CMD_AND_CMD_DATA(Dispatch, DISPATCH_CMD_DATA_MEMBER){};

#define SET_PIPELINE_CMD_DATA_MEMBER(X) X(ObjectId, pipelineId)

DAWN_REPLAY_MAKE_RENDER_PASS_CMD_AND_CMD_DATA(SetPipeline, SET_PIPELINE_CMD_DATA_MEMBER){};

DAWN_REPLAY_MAKE_RENDER_PASS_CMD_AND_CMD_DATA(SetBindGroup, SET_BIND_GROUP_CMD_DATA_MEMBER){};

#define SET_VERTEX_BUFFER_CMD_DATA_MEMBER(X) \
    X(uint32_t, slot)                        \
    X(ObjectId, bufferId)                    \
    X(uint64_t, offset)                      \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_RENDER_PASS_CMD_AND_CMD_DATA(SetVertexBuffer, SET_VERTEX_BUFFER_CMD_DATA_MEMBER){};

#define DRAW_CMD_DATA_MEMBER(X) \
    X(uint32_t, vertexCount)    \
    X(uint32_t, instanceCount)  \
    X(uint32_t, firstVertex)    \
    X(uint32_t, firstInstance)

DAWN_REPLAY_MAKE_RENDER_PASS_CMD_AND_CMD_DATA(Draw, DRAW_CMD_DATA_MEMBER){};

}  // namespace schema

#endif  // SRC_DAWN_SERIALIZATION_SCHEMA_H_
