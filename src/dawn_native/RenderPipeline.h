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

#ifndef DAWNNATIVE_RENDERPIPELINE_H_
#define DAWNNATIVE_RENDERPIPELINE_H_

#include "common/TypedInteger.h"
#include "dawn_native/AttachmentState.h"
#include "dawn_native/IntegerTypes.h"
#include "dawn_native/Pipeline.h"

#include "dawn_native/dawn_platform.h"

#include <array>
#include <bitset>

namespace dawn_native {

    class DeviceBase;

    MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                                const RenderPipelineDescriptor* descriptor);
    size_t IndexFormatSize(wgpu::IndexFormat format);
    uint32_t VertexFormatNumComponents(wgpu::VertexFormat format);
    size_t VertexFormatComponentSize(wgpu::VertexFormat format);
    size_t VertexFormatSize(wgpu::VertexFormat format);
    bool IsStripPrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology);

    bool StencilTestEnabled(const DepthStencilStateDescriptor* mDepthStencilState);
    bool BlendEnabled(const ColorStateDescriptor* mColorState);

    struct VertexAttributeInfo {
        wgpu::VertexFormat format;
        uint64_t offset;
        VertexAttributeLocation shaderLocation;
        VertexBufferSlot vertexBufferSlot;
    };

    struct VertexBufferInfo {
        uint64_t arrayStride;
        wgpu::InputStepMode stepMode;
    };

    class RenderPipelineBase : public PipelineBase {
      public:
        RenderPipelineBase(DeviceBase* device, const RenderPipelineDescriptor* descriptor);
        ~RenderPipelineBase() override;

        static RenderPipelineBase* MakeError(DeviceBase* device);

        const VertexStateDescriptor* GetVertexStateDescriptor() const;
        const ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes>&
        GetAttributeLocationsUsed() const;
        const VertexAttributeInfo& GetAttribute(VertexAttributeLocation location) const;
        const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>& GetVertexBufferSlotsUsed() const;
        const VertexBufferInfo& GetVertexBuffer(VertexBufferSlot slot) const;

        const ColorStateDescriptor* GetColorStateDescriptor(
            ColorAttachmentIndex attachmentSlot) const;
        const DepthStencilStateDescriptor* GetDepthStencilStateDescriptor() const;
        wgpu::PrimitiveTopology GetPrimitiveTopology() const;
        wgpu::CullMode GetCullMode() const;
        wgpu::FrontFace GetFrontFace() const;
        bool IsDepthBiasEnabled() const;
        int32_t GetDepthBias() const;
        float GetDepthBiasSlopeScale() const;
        float GetDepthBiasClamp() const;

        ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> GetColorAttachmentsMask() const;
        bool HasDepthStencilAttachment() const;
        wgpu::TextureFormat GetColorAttachmentFormat(ColorAttachmentIndex attachment) const;
        wgpu::TextureFormat GetDepthStencilFormat() const;
        uint32_t GetSampleCount() const;
        uint32_t GetSampleMask() const;
        bool IsAlphaToCoverageEnabled() const;

        const AttachmentState* GetAttachmentState() const;

        // Functions necessary for the unordered_set<RenderPipelineBase*>-based cache.
        size_t ComputeContentHash() override;

        struct EqualityFunc {
            bool operator()(const RenderPipelineBase* a, const RenderPipelineBase* b) const;
        };

      private:
        RenderPipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        // Vertex state
        VertexStateDescriptor mVertexState;
        ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes> mAttributeLocationsUsed;
        ityp::array<VertexAttributeLocation, VertexAttributeInfo, kMaxVertexAttributes>
            mAttributeInfos;
        ityp::bitset<VertexBufferSlot, kMaxVertexBuffers> mVertexBufferSlotsUsed;
        ityp::array<VertexBufferSlot, VertexBufferInfo, kMaxVertexBuffers> mVertexBufferInfos;

        // Attachments
        Ref<AttachmentState> mAttachmentState;
        DepthStencilStateDescriptor mDepthStencilState;
        ityp::array<ColorAttachmentIndex, ColorStateDescriptor, kMaxColorAttachments> mColorStates;

        // Other state
        wgpu::PrimitiveTopology mPrimitiveTopology;
        RasterizationStateDescriptor mRasterizationState;
        uint32_t mSampleMask;
        bool mAlphaToCoverageEnabled;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_RENDERPIPELINE_H_
