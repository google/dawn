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

#include "backend/Framebuffer.h"

#include "backend/Buffer.h"
#include "backend/Device.h"
#include "backend/RenderPass.h"
#include "backend/Texture.h"
#include "common/Assert.h"

namespace backend {

    // Framebuffer

    FramebufferBase::FramebufferBase(FramebufferBuilder* builder)
        : mDevice(builder->mDevice), mRenderPass(std::move(builder->mRenderPass)),
        mWidth(builder->mWidth), mHeight(builder->mHeight), mTextureViews(std::move(builder->mTextureViews)),
        mClearColors(mTextureViews.size()), mClearDepthStencils(mTextureViews.size()) {
    }

    DeviceBase* FramebufferBase::GetDevice() {
        return mDevice;
    }

    RenderPassBase* FramebufferBase::GetRenderPass() {
        return mRenderPass.Get();
    }

    TextureViewBase* FramebufferBase::GetTextureView(uint32_t attachmentSlot) {
        ASSERT(attachmentSlot < mTextureViews.size());
        return mTextureViews[attachmentSlot].Get();
    }

    FramebufferBase::ClearColor FramebufferBase::GetClearColor(uint32_t attachmentSlot) {
        ASSERT(attachmentSlot < mClearColors.size());
        return mClearColors[attachmentSlot];
    }

    FramebufferBase::ClearDepthStencil FramebufferBase::GetClearDepthStencil(uint32_t attachmentSlot) {
        ASSERT(attachmentSlot < mClearDepthStencils.size());
        return mClearDepthStencils[attachmentSlot];
    }

    uint32_t FramebufferBase::GetWidth() const {
        return mWidth;
    }

    uint32_t FramebufferBase::GetHeight() const {
        return mHeight;
    }

    void FramebufferBase::AttachmentSetClearColor(uint32_t attachmentSlot, float clearR, float clearG, float clearB, float clearA) {
        if (attachmentSlot >= mRenderPass->GetAttachmentCount()) {
            mDevice->HandleError("Framebuffer attachment out of bounds");
            return;
        }
        ASSERT(attachmentSlot < mClearColors.size());
        auto& c = mClearColors[attachmentSlot];
        c.color[0] = clearR;
        c.color[1] = clearG;
        c.color[2] = clearB;
        c.color[3] = clearA;
    }

    void FramebufferBase::AttachmentSetClearDepthStencil(uint32_t attachmentSlot, float clearDepth, uint32_t clearStencil) {
        if (attachmentSlot >= mRenderPass->GetAttachmentCount()) {
            mDevice->HandleError("Framebuffer attachment out of bounds");
            return;
        }
        ASSERT(attachmentSlot < mClearDepthStencils.size());
        auto& c = mClearDepthStencils[attachmentSlot];
        c.depth = clearDepth;
        c.stencil = clearStencil;
    }

    // FramebufferBuilder

    enum FramebufferSetProperties {
        FRAMEBUFFER_PROPERTY_RENDER_PASS = 0x1,
        FRAMEBUFFER_PROPERTY_DIMENSIONS = 0x2,
    };

    FramebufferBuilder::FramebufferBuilder(DeviceBase* device)
        : Builder(device) {
    }

    FramebufferBase* FramebufferBuilder::GetResultImpl() {
        constexpr int requiredProperties = FRAMEBUFFER_PROPERTY_RENDER_PASS | FRAMEBUFFER_PROPERTY_DIMENSIONS;
        if ((mPropertiesSet & requiredProperties) != requiredProperties) {
            HandleError("Framebuffer missing properties");
            return nullptr;
        }

        for (auto& textureView : mTextureViews) {
            if (!textureView) {
                HandleError("Framebuffer attachment not set");
                return nullptr;
            }

            // TODO(cwallez@chromium.org): Adjust for the mip-level once that is supported.
            if (textureView->GetTexture()->GetWidth() != mWidth ||
                textureView->GetTexture()->GetHeight() != mHeight) {
                HandleError("Framebuffer size doesn't match attachment size");
                return nullptr;
            }
        }

        return mDevice->CreateFramebuffer(this);
    }

    void FramebufferBuilder::SetRenderPass(RenderPassBase* renderPass) {
        if ((mPropertiesSet & FRAMEBUFFER_PROPERTY_RENDER_PASS) != 0) {
            HandleError("Framebuffer render pass property set multiple times");
            return;
        }
        // TODO(kainino@chromium.org): null checks should not be necessary
        if (renderPass == nullptr) {
            HandleError("Render pass invalid");
            return;
        }

        mRenderPass = renderPass;
        mTextureViews.resize(renderPass->GetAttachmentCount());
        mPropertiesSet |= FRAMEBUFFER_PROPERTY_RENDER_PASS;
    }

    void FramebufferBuilder::SetDimensions(uint32_t width, uint32_t height) {
        if ((mPropertiesSet & FRAMEBUFFER_PROPERTY_DIMENSIONS) != 0) {
            HandleError("Framebuffer dimensions property set multiple times");
            return;
        }

        mWidth = width;
        mHeight = height;
        mPropertiesSet |= FRAMEBUFFER_PROPERTY_DIMENSIONS;
    }

    void FramebufferBuilder::SetAttachment(uint32_t attachmentSlot, TextureViewBase* textureView) {
        if ((mPropertiesSet & FRAMEBUFFER_PROPERTY_RENDER_PASS) == 0) {
            HandleError("Render pass must be set before framebuffer attachments");
            return;
        }
        if (attachmentSlot >= mTextureViews.size()) {
            HandleError("Attachment slot out of bounds");
            return;
        }
        if (mTextureViews[attachmentSlot]) {
            HandleError("Framebuffer attachment[i] set multiple times");
            return;
        }
        const auto& attachmentInfo = mRenderPass->GetAttachmentInfo(attachmentSlot);
        const auto* texture = textureView->GetTexture();
        if (attachmentInfo.format != texture->GetFormat()) {
            HandleError("Texture format does not match attachment format");
            return;
        }
        // TODO(kainino@chromium.org): also check attachment samples, etc.

        mTextureViews[attachmentSlot] = textureView;
    }
}
