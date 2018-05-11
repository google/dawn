// Copyright 2017 The NXT Authors
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

#ifndef BACKEND_RENDERPIPELINE_H_
#define BACKEND_RENDERPIPELINE_H_

#include "backend/BlendState.h"
#include "backend/DepthStencilState.h"
#include "backend/InputState.h"
#include "backend/Pipeline.h"

#include "nxt/nxtcpp.h"

#include <array>
#include <bitset>

namespace backend {

    class RenderPipelineBase : public RefCounted, public PipelineBase {
      public:
        RenderPipelineBase(RenderPipelineBuilder* builder);

        BlendStateBase* GetBlendState(uint32_t attachmentSlot);
        DepthStencilStateBase* GetDepthStencilState();
        nxt::IndexFormat GetIndexFormat() const;
        InputStateBase* GetInputState();
        nxt::PrimitiveTopology GetPrimitiveTopology() const;

        std::bitset<kMaxColorAttachments> GetColorAttachmentsMask() const;
        bool HasDepthStencilAttachment() const;
        nxt::TextureFormat GetColorAttachmentFormat(uint32_t attachment) const;
        nxt::TextureFormat GetDepthStencilFormat() const;

        // A pipeline can be used in a render pass if its attachment info matches the actual
        // attachments in the render pass. This returns whether it is the case.
        bool IsCompatibleWith(const RenderPassDescriptorBase* renderPass) const;

      private:
        Ref<DepthStencilStateBase> mDepthStencilState;
        nxt::IndexFormat mIndexFormat;
        Ref<InputStateBase> mInputState;
        nxt::PrimitiveTopology mPrimitiveTopology;
        std::array<Ref<BlendStateBase>, kMaxColorAttachments> mBlendStates;

        std::bitset<kMaxColorAttachments> mColorAttachmentsSet;
        std::array<nxt::TextureFormat, kMaxColorAttachments> mColorAttachmentFormats;
        bool mDepthStencilFormatSet = false;
        nxt::TextureFormat mDepthStencilFormat;
    };

    class RenderPipelineBuilder : public Builder<RenderPipelineBase>, public PipelineBuilder {
      public:
        RenderPipelineBuilder(DeviceBase* device);

        // NXT API
        void SetColorAttachmentFormat(uint32_t attachmentSlot, nxt::TextureFormat format);
        void SetColorAttachmentBlendState(uint32_t attachmentSlot, BlendStateBase* blendState);
        void SetDepthStencilAttachmentFormat(nxt::TextureFormat format);
        void SetDepthStencilState(DepthStencilStateBase* depthStencilState);
        void SetPrimitiveTopology(nxt::PrimitiveTopology primitiveTopology);
        void SetIndexFormat(nxt::IndexFormat format);
        void SetInputState(InputStateBase* inputState);

      private:
        friend class RenderPipelineBase;

        RenderPipelineBase* GetResultImpl() override;

        Ref<DepthStencilStateBase> mDepthStencilState;
        Ref<InputStateBase> mInputState;
        // TODO(enga@google.com): Remove default when we validate that all required properties are
        // set
        nxt::PrimitiveTopology mPrimitiveTopology = nxt::PrimitiveTopology::TriangleList;
        nxt::IndexFormat mIndexFormat = nxt::IndexFormat::Uint32;
        std::bitset<kMaxColorAttachments> mBlendStatesSet;
        std::array<Ref<BlendStateBase>, kMaxColorAttachments> mBlendStates;
        std::bitset<kMaxColorAttachments> mColorAttachmentsSet;
        std::array<nxt::TextureFormat, kMaxColorAttachments> mColorAttachmentFormats;
        bool mDepthStencilFormatSet = false;
        nxt::TextureFormat mDepthStencilFormat;
    };

}  // namespace backend

#endif  // BACKEND_RENDERPIPELINE_H_
