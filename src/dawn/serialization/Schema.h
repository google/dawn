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

#define TEXEL_COPY_TEXTURE_INFO_MEMBER(X) \
    X(ObjectId, textureId)                \
    X(uint32_t, mipLevel)                 \
    X(Origin3D, origin)                   \
    X(wgpu::TextureAspect, aspect)

DAWN_REPLAY_SERIALIZABLE(struct, TexelCopyTextureInfo, TEXEL_COPY_TEXTURE_INFO_MEMBER){};

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

}  // namespace schema

#endif  // SRC_DAWN_SERIALIZATION_SCHEMA_H_
