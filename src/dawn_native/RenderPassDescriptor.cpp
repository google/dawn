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

#include "dawn_native/RenderPassDescriptor.h"

#include "common/Assert.h"
#include "common/BitSetIterator.h"
#include "dawn_native/Device.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    // RenderPassDescriptor

    RenderPassDescriptorBase::RenderPassDescriptorBase(RenderPassDescriptorBuilder* builder)
        : ObjectBase(builder->GetDevice()),
          mColorAttachmentsSet(builder->mColorAttachmentsSet),
          mColorAttachments(builder->mColorAttachments),
          mDepthStencilAttachmentSet(builder->mDepthStencilAttachmentSet),
          mDepthStencilAttachment(builder->mDepthStencilAttachment),
          mWidth(builder->mWidth),
          mHeight(builder->mHeight) {
    }

    std::bitset<kMaxColorAttachments> RenderPassDescriptorBase::GetColorAttachmentMask() const {
        return mColorAttachmentsSet;
    }

    bool RenderPassDescriptorBase::HasDepthStencilAttachment() const {
        return mDepthStencilAttachmentSet;
    }

    const RenderPassColorAttachmentInfo& RenderPassDescriptorBase::GetColorAttachment(
        uint32_t attachment) const {
        ASSERT(attachment < kMaxColorAttachments);
        ASSERT(mColorAttachmentsSet[attachment]);

        return mColorAttachments[attachment];
    }

    RenderPassColorAttachmentInfo& RenderPassDescriptorBase::GetColorAttachment(
        uint32_t attachment) {
        ASSERT(attachment < kMaxColorAttachments);
        ASSERT(mColorAttachmentsSet[attachment]);

        return mColorAttachments[attachment];
    }

    const RenderPassDepthStencilAttachmentInfo&
    RenderPassDescriptorBase::GetDepthStencilAttachment() const {
        ASSERT(mDepthStencilAttachmentSet);

        return mDepthStencilAttachment;
    }

    RenderPassDepthStencilAttachmentInfo& RenderPassDescriptorBase::GetDepthStencilAttachment() {
        ASSERT(mDepthStencilAttachmentSet);

        return mDepthStencilAttachment;
    }

    uint32_t RenderPassDescriptorBase::GetWidth() const {
        return mWidth;
    }

    uint32_t RenderPassDescriptorBase::GetHeight() const {
        return mHeight;
    }

    // RenderPassDescriptorBuilder

    RenderPassDescriptorBuilder::RenderPassDescriptorBuilder(DeviceBase* device) : Builder(device) {
    }

    bool RenderPassDescriptorBuilder::CheckArrayLayersAndLevelCountForAttachment(
        const TextureViewBase* textureView) {
        // Currently we do not support layered rendering.
        if (textureView->GetLayerCount() > 1) {
            HandleError(
                "The layer count of the texture view used as attachment cannot be greater than 1");
            return false;
        }

        if (textureView->GetLevelCount() > 1) {
            HandleError(
                "The mipmap level count of the texture view used as attachment cannot be greater "
                "than 1");
            return false;
        }
        return true;
    }

    RenderPassDescriptorBase* RenderPassDescriptorBuilder::GetResultImpl() {
        auto CheckOrSetSize = [this](const TextureViewBase* attachment) -> bool {
            uint32_t mipLevel = attachment->GetBaseMipLevel();
            if (this->mWidth == 0) {
                ASSERT(this->mHeight == 0);

                this->mWidth = attachment->GetTexture()->GetSize().width >> mipLevel;
                this->mHeight = attachment->GetTexture()->GetSize().height >> mipLevel;
                ASSERT(this->mWidth != 0 && this->mHeight != 0);

                return true;
            }

            ASSERT(this->mWidth != 0 && this->mHeight != 0);
            return this->mWidth == attachment->GetTexture()->GetSize().width >> mipLevel &&
                   this->mHeight == attachment->GetTexture()->GetSize().height >> mipLevel;
        };

        uint32_t attachmentCount = 0;
        for (uint32_t i : IterateBitSet(mColorAttachmentsSet)) {
            attachmentCount++;
            if (!CheckOrSetSize(mColorAttachments[i].view.Get())) {
                HandleError("Attachment size mismatch");
                return nullptr;
            }
        }

        if (mDepthStencilAttachmentSet) {
            attachmentCount++;
            if (!CheckOrSetSize(mDepthStencilAttachment.view.Get())) {
                HandleError("Attachment size mismatch");
                return nullptr;
            }
        }

        if (attachmentCount == 0) {
            HandleError("Should have at least one attachment");
            return nullptr;
        }

        return GetDevice()->CreateRenderPassDescriptor(this);
    }

    void RenderPassDescriptorBuilder::SetColorAttachments(
        uint32_t count,
        const RenderPassColorAttachmentDescriptor* attachments) {
        if (count > kMaxColorAttachments) {
            HandleError("Setting color attachments out of bounds");
            return;
        }

        for (uint32_t i = 0; i < count; ++i) {
            // TODO(jiawei.shao@intel.com): support resolve target for multisample color attachment.
            if (attachments[i].resolveTarget != nullptr) {
                HandleError("Resolve target is not supported now");
                return;
            }

            TextureViewBase* textureView = attachments[i].attachment;
            if (textureView == nullptr) {
                continue;
            }

            if (!IsColorRenderableTextureFormat(textureView->GetFormat())) {
                HandleError(
                    "The format of the texture view used as color attachment is not color "
                    "renderable");
                return;
            }

            if (!CheckArrayLayersAndLevelCountForAttachment(textureView)) {
                return;
            }

            // TODO(jiawei.shao@intel.com): set and make use of storeOp
            mColorAttachmentsSet.set(i);
            mColorAttachments[i].loadOp = attachments[i].loadOp;
            mColorAttachments[i].view = textureView;

            mColorAttachments[i].clearColor[0] = attachments[i].clearColor.r;
            mColorAttachments[i].clearColor[1] = attachments[i].clearColor.g;
            mColorAttachments[i].clearColor[2] = attachments[i].clearColor.b;
            mColorAttachments[i].clearColor[3] = attachments[i].clearColor.a;
        }
    }

    void RenderPassDescriptorBuilder::SetDepthStencilAttachment(
        const RenderPassDepthStencilAttachmentDescriptor* attachment) {
        TextureViewBase* textureView = attachment->attachment;
        if (textureView == nullptr) {
            HandleError("Texture view cannot be nullptr");
            return;
        }

        if (!TextureFormatHasDepthOrStencil(textureView->GetFormat())) {
            HandleError(
                "The format of the texture view used as depth stencil attachment is not a depth "
                "stencil format");
            return;
        }

        if (!CheckArrayLayersAndLevelCountForAttachment(textureView)) {
            return;
        }

        // TODO(jiawei.shao@intel.com): set and make use of depthStoreOp and stencilStoreOp
        mDepthStencilAttachmentSet = true;
        mDepthStencilAttachment.depthLoadOp = attachment->depthLoadOp;
        mDepthStencilAttachment.stencilLoadOp = attachment->stencilLoadOp;
        mDepthStencilAttachment.view = textureView;
        mDepthStencilAttachment.clearDepth = attachment->clearDepth;
        mDepthStencilAttachment.clearStencil = attachment->clearStencil;
    }

}  // namespace dawn_native
