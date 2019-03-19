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

#include "dawn_native/InputState.h"
#include "dawn_native/Pipeline.h"

#include "dawn_native/dawn_platform.h"

#include <array>
#include <bitset>

namespace dawn_native {

    struct BeginRenderPassCmd;

    class DeviceBase;

    MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                                const RenderPipelineDescriptor* descriptor);
    bool StencilTestEnabled(const DepthStencilStateDescriptor* mDepthStencilState);
    bool BlendEnabled(const ColorStateDescriptor* mColorState);

    class RenderPipelineBase : public PipelineBase {
      public:
        RenderPipelineBase(DeviceBase* device, const RenderPipelineDescriptor* descriptor);

        static RenderPipelineBase* MakeError(DeviceBase* device);

        const ColorStateDescriptor* GetColorStateDescriptor(uint32_t attachmentSlot);
        const DepthStencilStateDescriptor* GetDepthStencilStateDescriptor();
        dawn::IndexFormat GetIndexFormat() const;
        InputStateBase* GetInputState();
        dawn::PrimitiveTopology GetPrimitiveTopology() const;

        std::bitset<kMaxColorAttachments> GetColorAttachmentsMask() const;
        bool HasDepthStencilAttachment() const;
        dawn::TextureFormat GetColorAttachmentFormat(uint32_t attachment) const;
        dawn::TextureFormat GetDepthStencilFormat() const;

        // A pipeline can be used in a render pass if its attachment info matches the actual
        // attachments in the render pass. This returns whether it is the case.
        bool IsCompatibleWith(const BeginRenderPassCmd* renderPassCmd) const;

      private:
        RenderPipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        DepthStencilStateDescriptor mDepthStencilState;
        dawn::IndexFormat mIndexFormat;
        Ref<InputStateBase> mInputState;
        dawn::PrimitiveTopology mPrimitiveTopology;
        std::array<ColorStateDescriptor, kMaxColorAttachments> mColorStates;

        std::bitset<kMaxColorAttachments> mColorAttachmentsSet;
        bool mHasDepthStencilAttachment = false;

        uint32_t mSampleCount;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_RENDERPIPELINE_H_
