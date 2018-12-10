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

#include "dawn_native/BlendState.h"
#include "dawn_native/DepthStencilState.h"
#include "dawn_native/InputState.h"
#include "dawn_native/Pipeline.h"

#include "dawn_native/dawn_platform.h"

#include <array>
#include <bitset>

namespace dawn_native {

    class DeviceBase;

    MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                                const RenderPipelineDescriptor* descriptor);

    class RenderPipelineBase : public PipelineBase {
      public:
        RenderPipelineBase(DeviceBase* device, const RenderPipelineDescriptor* descriptor);

        BlendStateBase* GetBlendState(uint32_t attachmentSlot);
        DepthStencilStateBase* GetDepthStencilState();
        dawn::IndexFormat GetIndexFormat() const;
        InputStateBase* GetInputState();
        dawn::PrimitiveTopology GetPrimitiveTopology() const;

        std::bitset<kMaxColorAttachments> GetColorAttachmentsMask() const;
        bool HasDepthStencilAttachment() const;
        dawn::TextureFormat GetColorAttachmentFormat(uint32_t attachment) const;
        dawn::TextureFormat GetDepthStencilFormat() const;

        // A pipeline can be used in a render pass if its attachment info matches the actual
        // attachments in the render pass. This returns whether it is the case.
        bool IsCompatibleWith(const RenderPassDescriptorBase* renderPass) const;

      private:
        Ref<DepthStencilStateBase> mDepthStencilState;
        dawn::IndexFormat mIndexFormat;
        Ref<InputStateBase> mInputState;
        dawn::PrimitiveTopology mPrimitiveTopology;
        std::array<Ref<BlendStateBase>, kMaxColorAttachments> mBlendStates;

        std::bitset<kMaxColorAttachments> mColorAttachmentsSet;
        std::array<dawn::TextureFormat, kMaxColorAttachments> mColorAttachmentFormats;
        bool mHasDepthStencilAttachment = false;
        dawn::TextureFormat mDepthStencilFormat;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_RENDERPIPELINE_H_
