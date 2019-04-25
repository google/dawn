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

#include "dawn_native/Pipeline.h"

#include "dawn_native/dawn_platform.h"

#include <array>
#include <bitset>

namespace dawn_native {

    struct BeginRenderPassCmd;

    class DeviceBase;

    MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                                const RenderPipelineDescriptor* descriptor);
    size_t IndexFormatSize(dawn::IndexFormat format);
    uint32_t VertexFormatNumComponents(dawn::VertexFormat format);
    size_t VertexFormatComponentSize(dawn::VertexFormat format);
    size_t VertexFormatSize(dawn::VertexFormat format);

    bool StencilTestEnabled(const DepthStencilStateDescriptor* mDepthStencilState);
    bool BlendEnabled(const ColorStateDescriptor* mColorState);

    class RenderPipelineBase : public PipelineBase {
      public:
        RenderPipelineBase(DeviceBase* device, const RenderPipelineDescriptor* descriptor);

        static RenderPipelineBase* MakeError(DeviceBase* device);

        const InputStateDescriptor* GetInputStateDescriptor() const;
        const std::bitset<kMaxVertexAttributes>& GetAttributesSetMask() const;
        const VertexAttributeDescriptor& GetAttribute(uint32_t location) const;
        const std::bitset<kMaxVertexInputs>& GetInputsSetMask() const;
        const VertexInputDescriptor& GetInput(uint32_t slot) const;

        const ColorStateDescriptor* GetColorStateDescriptor(uint32_t attachmentSlot);
        const DepthStencilStateDescriptor* GetDepthStencilStateDescriptor();
        dawn::PrimitiveTopology GetPrimitiveTopology() const;
        dawn::CullMode GetCullMode() const;
        dawn::FrontFace GetFrontFace() const;

        std::bitset<kMaxColorAttachments> GetColorAttachmentsMask() const;
        bool HasDepthStencilAttachment() const;
        dawn::TextureFormat GetColorAttachmentFormat(uint32_t attachment) const;
        dawn::TextureFormat GetDepthStencilFormat() const;
        uint32_t GetSampleCount() const;

        // A pipeline can be used in a render pass if its attachment info matches the actual
        // attachments in the render pass. This returns whether it is the case.
        bool IsCompatibleWith(const BeginRenderPassCmd* renderPassCmd) const;
        std::bitset<kMaxVertexAttributes> GetAttributesUsingInput(uint32_t slot) const;
        std::array<std::bitset<kMaxVertexAttributes>, kMaxVertexInputs> attributesUsingInput;

      private:
        RenderPipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        InputStateDescriptor mInputState;
        std::bitset<kMaxVertexAttributes> mAttributesSetMask;
        std::array<VertexAttributeDescriptor, kMaxVertexAttributes> mAttributeInfos;
        std::bitset<kMaxVertexInputs> mInputsSetMask;
        std::array<VertexInputDescriptor, kMaxVertexInputs> mInputInfos;
        dawn::PrimitiveTopology mPrimitiveTopology;
        RasterizationStateDescriptor mRasterizationState;
        DepthStencilStateDescriptor mDepthStencilState;
        std::array<ColorStateDescriptor, kMaxColorAttachments> mColorStates;

        std::bitset<kMaxColorAttachments> mColorAttachmentsSet;
        bool mHasDepthStencilAttachment = false;

        uint32_t mSampleCount;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_RENDERPIPELINE_H_
