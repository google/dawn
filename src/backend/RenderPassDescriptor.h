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

#ifndef BACKEND_RENDERPASSDESCRIPTOR_H_
#define BACKEND_RENDERPASSDESCRIPTOR_H_

#include "backend/Builder.h"
#include "backend/Forward.h"
#include "backend/RefCounted.h"
#include "common/Constants.h"

#include "nxt/nxtcpp.h"

#include <array>
#include <bitset>
#include <vector>

namespace backend {

    struct RenderPassColorAttachmentInfo {
        nxt::LoadOp loadOp;
        std::array<float, 4> clearColor = {{0.0f, 0.0f, 0.0f, 0.0f}};
        Ref<TextureViewBase> view;
    };

    struct RenderPassDepthStencilAttachmentInfo {
        nxt::LoadOp depthLoadOp;
        nxt::LoadOp stencilLoadOp;
        float clearDepth = 1.0f;
        uint32_t clearStencil = 0;
        Ref<TextureViewBase> view;
    };

    // RenderPassDescriptor contains the list of attachments for a renderpass along with data such
    // as the load operation and the clear values for the attachments.

    class RenderPassDescriptorBase : public RefCounted {
      public:
        RenderPassDescriptorBase(RenderPassDescriptorBuilder* builder);

        std::bitset<kMaxColorAttachments> GetColorAttachmentMask() const;
        bool HasDepthStencilAttachment() const;

        const RenderPassColorAttachmentInfo& GetColorAttachment(uint32_t attachment) const;
        RenderPassColorAttachmentInfo& GetColorAttachment(uint32_t attachment);
        const RenderPassDepthStencilAttachmentInfo& GetDepthStencilAttachment() const;
        RenderPassDepthStencilAttachmentInfo& GetDepthStencilAttachment();

        // All attachments of the render pass have the same size, these return that size.
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

      private:
        std::bitset<kMaxColorAttachments> mColorAttachmentsSet;
        std::array<RenderPassColorAttachmentInfo, kMaxColorAttachments> mColorAttachments;

        bool mDepthStencilAttachmentSet;
        RenderPassDepthStencilAttachmentInfo mDepthStencilAttachment;

        uint32_t mWidth;
        uint32_t mHeight;
    };

    class RenderPassDescriptorBuilder : public Builder<RenderPassDescriptorBase> {
      public:
        RenderPassDescriptorBuilder(DeviceBase* device);

        // NXT API
        RenderPassDescriptorBase* GetResultImpl() override;
        void SetColorAttachment(uint32_t attachment,
                                TextureViewBase* textureView,
                                nxt::LoadOp loadOp);
        void SetColorAttachmentClearColor(uint32_t attachment,
                                          float clearR,
                                          float clearG,
                                          float clearB,
                                          float clearA);
        void SetDepthStencilAttachment(TextureViewBase* textureView,
                                       nxt::LoadOp depthLoadOp,
                                       nxt::LoadOp stencilLoadOp);
        void SetDepthStencilAttachmentClearValue(float clearDepth, uint32_t clearStencil);

      private:
        friend class RenderPassDescriptorBase;

        std::bitset<kMaxColorAttachments> mColorAttachmentsSet;
        std::array<RenderPassColorAttachmentInfo, kMaxColorAttachments> mColorAttachments;

        bool mDepthStencilAttachmentSet = false;
        RenderPassDepthStencilAttachmentInfo mDepthStencilAttachment;

        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
    };

}  // namespace backend

#endif  // BACKEND_RENDERPASS_H_
