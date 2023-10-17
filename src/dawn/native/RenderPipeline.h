// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_RENDERPIPELINE_H_
#define SRC_DAWN_NATIVE_RENDERPIPELINE_H_

#include <array>
#include <bitset>
#include <vector>

#include "dawn/common/ContentLessObjectCacheable.h"
#include "dawn/common/TypedInteger.h"
#include "dawn/native/AttachmentState.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/Pipeline.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

enum class VertexFormatBaseType {
    Float,
    Uint,
    Sint,
};

struct VertexFormatInfo {
    wgpu::VertexFormat format;
    uint32_t byteSize;
    uint32_t componentCount;
    VertexFormatBaseType baseType;
};

const VertexFormatInfo& GetVertexFormatInfo(wgpu::VertexFormat format);

class DeviceBase;

MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                            const RenderPipelineDescriptor* descriptor);

std::vector<StageAndDescriptor> GetRenderStagesAndSetPlaceholderShader(
    DeviceBase* device,
    const RenderPipelineDescriptor* descriptor);

size_t IndexFormatSize(wgpu::IndexFormat format);

bool IsStripPrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology);

bool StencilTestEnabled(const DepthStencilState* depthStencil);

struct VertexAttributeInfo {
    wgpu::VertexFormat format;
    uint64_t offset;
    VertexAttributeLocation shaderLocation;
    VertexBufferSlot vertexBufferSlot;
};

struct VertexBufferInfo {
    uint64_t arrayStride;
    wgpu::VertexStepMode stepMode;
    uint16_t usedBytesInStride;
    // As indicated in the spec, the lastStride is max(attribute.offset +
    // sizeof(attribute.format)) for each attribute in the buffer[slot]
    uint64_t lastStride;
};

class RenderPipelineBase : public PipelineBase,
                           public ContentLessObjectCacheable<RenderPipelineBase> {
  public:
    RenderPipelineBase(DeviceBase* device, const RenderPipelineDescriptor* descriptor);
    ~RenderPipelineBase() override;

    static RenderPipelineBase* MakeError(DeviceBase* device, const char* label);

    ObjectType GetType() const override;

    const ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes>& GetAttributeLocationsUsed()
        const;
    const VertexAttributeInfo& GetAttribute(VertexAttributeLocation location) const;
    const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>& GetVertexBufferSlotsUsed() const;
    const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>&
    GetVertexBufferSlotsUsedAsVertexBuffer() const;
    const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>&
    GetVertexBufferSlotsUsedAsInstanceBuffer() const;
    const VertexBufferInfo& GetVertexBuffer(VertexBufferSlot slot) const;
    uint32_t GetVertexBufferCount() const;

    const ColorTargetState* GetColorTargetState(ColorAttachmentIndex attachmentSlot) const;
    const DepthStencilState* GetDepthStencilState() const;
    wgpu::PrimitiveTopology GetPrimitiveTopology() const;
    wgpu::IndexFormat GetStripIndexFormat() const;
    wgpu::CullMode GetCullMode() const;
    wgpu::FrontFace GetFrontFace() const;
    bool IsDepthBiasEnabled() const;
    int32_t GetDepthBias() const;
    float GetDepthBiasSlopeScale() const;
    float GetDepthBiasClamp() const;
    bool HasUnclippedDepth() const;

    ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> GetColorAttachmentsMask() const;
    bool HasDepthStencilAttachment() const;
    wgpu::TextureFormat GetColorAttachmentFormat(ColorAttachmentIndex attachment) const;
    wgpu::TextureFormat GetDepthStencilFormat() const;
    uint32_t GetSampleCount() const;
    uint32_t GetSampleMask() const;
    bool IsAlphaToCoverageEnabled() const;
    bool WritesDepth() const;
    bool WritesStencil() const;
    bool UsesFragDepth() const;

    const AttachmentState* GetAttachmentState() const;

    // Functions necessary for the unordered_set<RenderPipelineBase*>-based cache.
    size_t ComputeContentHash() override;

    struct EqualityFunc {
        bool operator()(const RenderPipelineBase* a, const RenderPipelineBase* b) const;
    };

  protected:
    void DestroyImpl() override;

  private:
    RenderPipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label);

    // Vertex state
    uint32_t mVertexBufferCount;
    ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes> mAttributeLocationsUsed;
    ityp::array<VertexAttributeLocation, VertexAttributeInfo, kMaxVertexAttributes> mAttributeInfos;
    ityp::bitset<VertexBufferSlot, kMaxVertexBuffers> mVertexBufferSlotsUsed;
    ityp::bitset<VertexBufferSlot, kMaxVertexBuffers> mVertexBufferSlotsUsedAsVertexBuffer;
    ityp::bitset<VertexBufferSlot, kMaxVertexBuffers> mVertexBufferSlotsUsedAsInstanceBuffer;
    ityp::array<VertexBufferSlot, VertexBufferInfo, kMaxVertexBuffers> mVertexBufferInfos;

    // Attachments
    Ref<AttachmentState> mAttachmentState;
    ityp::array<ColorAttachmentIndex, ColorTargetState, kMaxColorAttachments> mTargets;
    ityp::array<ColorAttachmentIndex, BlendState, kMaxColorAttachments> mTargetBlend;

    // Other state
    PrimitiveState mPrimitive;
    DepthStencilState mDepthStencil;
    MultisampleState mMultisample;
    bool mUnclippedDepth = false;
    bool mWritesDepth = false;
    bool mWritesStencil = false;
    bool mUsesFragDepth = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RENDERPIPELINE_H_
