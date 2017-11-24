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

#include "backend/RenderPass.h"

#include "backend/Buffer.h"
#include "backend/Device.h"
#include "backend/Texture.h"
#include "common/Assert.h"
#include "common/BitSetIterator.h"

namespace backend {

    // RenderPass

    RenderPassBase::RenderPassBase(RenderPassBuilder* builder)
        : mAttachments(std::move(builder->mAttachments)),
          mSubpasses(std::move(builder->mSubpasses)) {
        for (uint32_t s = 0; s < GetSubpassCount(); ++s) {
            const auto& subpass = GetSubpassInfo(s);
            for (auto location : IterateBitSet(subpass.colorAttachmentsSet)) {
                auto attachmentSlot = subpass.colorAttachments[location];
                auto& firstSubpass = mAttachments[attachmentSlot].firstSubpass;
                if (firstSubpass == UINT32_MAX) {
                    firstSubpass = s;
                }
            }
            if (subpass.depthStencilAttachmentSet) {
                auto attachmentSlot = subpass.depthStencilAttachment;
                auto& firstSubpass = mAttachments[attachmentSlot].firstSubpass;
                if (firstSubpass == UINT32_MAX) {
                    firstSubpass = s;
                }
            }
        }
    }

    uint32_t RenderPassBase::GetAttachmentCount() const {
        return static_cast<uint32_t>(mAttachments.size());
    }

    const RenderPassBase::AttachmentInfo& RenderPassBase::GetAttachmentInfo(
        uint32_t attachment) const {
        ASSERT(attachment < mAttachments.size());
        return mAttachments[attachment];
    }

    uint32_t RenderPassBase::GetSubpassCount() const {
        return static_cast<uint32_t>(mSubpasses.size());
    }

    const RenderPassBase::SubpassInfo& RenderPassBase::GetSubpassInfo(uint32_t subpass) const {
        ASSERT(subpass < mSubpasses.size());
        return mSubpasses[subpass];
    }

    bool RenderPassBase::IsCompatibleWith(const RenderPassBase* other) const {
        // TODO(kainino@chromium.org): This check is overly strict; need actual
        // compatibility checking (different load and store ops, etc.)
        return other == this;
    }

    // RenderPassBuilder

    enum RenderPassSetProperties {
        RENDERPASS_PROPERTY_ATTACHMENT_COUNT = 0x1,
        RENDERPASS_PROPERTY_SUBPASS_COUNT = 0x2,
    };

    RenderPassBuilder::RenderPassBuilder(DeviceBase* device) : Builder(device), mSubpasses(1) {
    }

    RenderPassBase* RenderPassBuilder::GetResultImpl() {
        constexpr int requiredProperties =
            RENDERPASS_PROPERTY_ATTACHMENT_COUNT | RENDERPASS_PROPERTY_SUBPASS_COUNT;
        if ((mPropertiesSet & requiredProperties) != requiredProperties) {
            HandleError("Render pass missing properties");
            return nullptr;
        }

        for (const auto& prop : mAttachmentProperties) {
            if (!prop.all()) {
                HandleError("A render pass attachment is missing some property");
                return nullptr;
            }
        }

        for (const auto& subpass : mSubpasses) {
            for (unsigned int location : IterateBitSet(subpass.colorAttachmentsSet)) {
                uint32_t slot = subpass.colorAttachments[location];
                if (TextureFormatHasDepthOrStencil(mAttachments[slot].format)) {
                    HandleError("Render pass color attachment is not of a color format");
                    return nullptr;
                }
            }
            if (subpass.depthStencilAttachmentSet) {
                uint32_t slot = subpass.depthStencilAttachment;
                if (!TextureFormatHasDepthOrStencil(mAttachments[slot].format)) {
                    HandleError(
                        "Render pass depth/stencil attachment is not of a depth/stencil format");
                    return nullptr;
                }
            }
        }

        return mDevice->CreateRenderPass(this);
    }

    void RenderPassBuilder::SetAttachmentCount(uint32_t attachmentCount) {
        if ((mPropertiesSet & RENDERPASS_PROPERTY_ATTACHMENT_COUNT) != 0) {
            HandleError("Render pass attachment count property set multiple times");
            return;
        }

        mAttachmentProperties.resize(attachmentCount);
        mAttachments.resize(attachmentCount);
        mPropertiesSet |= RENDERPASS_PROPERTY_ATTACHMENT_COUNT;
    }

    void RenderPassBuilder::AttachmentSetFormat(uint32_t attachmentSlot,
                                                nxt::TextureFormat format) {
        if ((mPropertiesSet & RENDERPASS_PROPERTY_ATTACHMENT_COUNT) == 0) {
            HandleError("Render pass attachment count not set yet");
            return;
        }
        if (attachmentSlot >= mAttachments.size()) {
            HandleError("Render pass attachment slot out of bounds");
            return;
        }
        if (mAttachmentProperties[attachmentSlot][ATTACHMENT_PROPERTY_FORMAT]) {
            HandleError("Render pass attachment format already set");
            return;
        }

        mAttachments[attachmentSlot].format = format;
        mAttachmentProperties[attachmentSlot].set(ATTACHMENT_PROPERTY_FORMAT);
    }

    void RenderPassBuilder::AttachmentSetColorLoadOp(uint32_t attachmentSlot, nxt::LoadOp op) {
        if ((mPropertiesSet & RENDERPASS_PROPERTY_ATTACHMENT_COUNT) == 0) {
            HandleError("Render pass attachment count not set yet");
            return;
        }
        if (attachmentSlot >= mAttachments.size()) {
            HandleError("Render pass attachment slot out of bounds");
            return;
        }

        mAttachments[attachmentSlot].colorLoadOp = op;
    }

    void RenderPassBuilder::AttachmentSetDepthStencilLoadOps(uint32_t attachmentSlot,
                                                             nxt::LoadOp depthOp,
                                                             nxt::LoadOp stencilOp) {
        if ((mPropertiesSet & RENDERPASS_PROPERTY_ATTACHMENT_COUNT) == 0) {
            HandleError("Render pass attachment count not set yet");
            return;
        }
        if (attachmentSlot >= mAttachments.size()) {
            HandleError("Render pass attachment slot out of bounds");
            return;
        }

        mAttachments[attachmentSlot].depthLoadOp = depthOp;
        mAttachments[attachmentSlot].stencilLoadOp = stencilOp;
    }

    void RenderPassBuilder::SetSubpassCount(uint32_t subpassCount) {
        if ((mPropertiesSet & RENDERPASS_PROPERTY_SUBPASS_COUNT) != 0) {
            HandleError("Render pass subpass count property set multiple times");
            return;
        }
        if (subpassCount < 1) {
            HandleError("Render pass cannot have fewer than one subpass");
            return;
        }

        mSubpasses.resize(subpassCount);
        mPropertiesSet |= RENDERPASS_PROPERTY_SUBPASS_COUNT;
    }

    void RenderPassBuilder::SubpassSetColorAttachment(uint32_t subpass,
                                                      uint32_t outputAttachmentLocation,
                                                      uint32_t attachmentSlot) {
        if ((mPropertiesSet & RENDERPASS_PROPERTY_SUBPASS_COUNT) == 0) {
            HandleError("Render pass subpass count not set yet");
            return;
        }
        if ((mPropertiesSet & RENDERPASS_PROPERTY_ATTACHMENT_COUNT) == 0) {
            HandleError("Render pass attachment count not set yet");
            return;
        }
        if (subpass >= mSubpasses.size()) {
            HandleError("Subpass index out of bounds");
            return;
        }
        if (outputAttachmentLocation >= kMaxColorAttachments) {
            HandleError("Subpass output attachment location out of bounds");
            return;
        }
        if (attachmentSlot >= mAttachments.size()) {
            HandleError("Subpass attachment slot out of bounds");
            return;
        }
        if (mSubpasses[subpass].colorAttachmentsSet[outputAttachmentLocation]) {
            HandleError("Subpass color attachment already set");
            return;
        }

        mSubpasses[subpass].colorAttachmentsSet.set(outputAttachmentLocation);
        mSubpasses[subpass].colorAttachments[outputAttachmentLocation] = attachmentSlot;
    }

    void RenderPassBuilder::SubpassSetDepthStencilAttachment(uint32_t subpass,
                                                             uint32_t attachmentSlot) {
        if ((mPropertiesSet & RENDERPASS_PROPERTY_SUBPASS_COUNT) == 0) {
            HandleError("Render pass subpass count not set yet");
            return;
        }
        if ((mPropertiesSet & RENDERPASS_PROPERTY_ATTACHMENT_COUNT) == 0) {
            HandleError("Render pass attachment count not set yet");
            return;
        }
        if (subpass >= mSubpasses.size()) {
            HandleError("Subpass index out of bounds");
            return;
        }
        if (attachmentSlot >= mAttachments.size()) {
            HandleError("Subpass attachment slot out of bounds");
            return;
        }
        if (mSubpasses[subpass].depthStencilAttachmentSet) {
            HandleError("Subpass depth-stencil attachment already set");
            return;
        }

        mSubpasses[subpass].depthStencilAttachmentSet = true;
        mSubpasses[subpass].depthStencilAttachment = attachmentSlot;
    }

}  // namespace backend
