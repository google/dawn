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
        : device(builder->device), renderPass(std::move(builder->renderPass)),
        width(builder->width), height(builder->height), textureViews(std::move(builder->textureViews)),
        clearColors(textureViews.size()), clearDepthStencils(textureViews.size()) {
    }

    DeviceBase* FramebufferBase::GetDevice() {
        return device;
    }

    RenderPassBase* FramebufferBase::GetRenderPass() {
        return renderPass.Get();
    }

    TextureViewBase* FramebufferBase::GetTextureView(uint32_t attachmentSlot) {
        ASSERT(attachmentSlot < textureViews.size());
        return textureViews[attachmentSlot].Get();
    }

    FramebufferBase::ClearColor FramebufferBase::GetClearColor(uint32_t attachmentSlot) {
        ASSERT(attachmentSlot < clearColors.size());
        return clearColors[attachmentSlot];
    }

    FramebufferBase::ClearDepthStencil FramebufferBase::GetClearDepthStencil(uint32_t attachmentSlot) {
        ASSERT(attachmentSlot < clearDepthStencils.size());
        return clearDepthStencils[attachmentSlot];
    }

    uint32_t FramebufferBase::GetWidth() const {
        return width;
    }

    uint32_t FramebufferBase::GetHeight() const {
        return height;
    }

    void FramebufferBase::AttachmentSetClearColor(uint32_t attachmentSlot, float clearR, float clearG, float clearB, float clearA) {
        if (attachmentSlot >= renderPass->GetAttachmentCount()) {
            device->HandleError("Framebuffer attachment out of bounds");
            return;
        }
        ASSERT(attachmentSlot < clearColors.size());
        auto& c = clearColors[attachmentSlot];
        c.color[0] = clearR;
        c.color[1] = clearG;
        c.color[2] = clearB;
        c.color[3] = clearA;
    }

    void FramebufferBase::AttachmentSetClearDepthStencil(uint32_t attachmentSlot, float clearDepth, uint32_t clearStencil) {
        if (attachmentSlot >= renderPass->GetAttachmentCount()) {
            device->HandleError("Framebuffer attachment out of bounds");
            return;
        }
        ASSERT(attachmentSlot < clearDepthStencils.size());
        auto& c = clearDepthStencils[attachmentSlot];
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
        if ((propertiesSet & requiredProperties) != requiredProperties) {
            HandleError("Framebuffer missing properties");
            return nullptr;
        }

        for (auto& textureView : textureViews) {
            if (!textureView) {
                HandleError("Framebuffer attachment not set");
                return nullptr;
            }

            // TODO(cwallez@chromium.org): Adjust for the mip-level once that is supported.
            if (textureView->GetTexture()->GetWidth() != width ||
                textureView->GetTexture()->GetHeight() != height) {
                HandleError("Framebuffer size doesn't match attachment size");
                return nullptr;
            }
        }

        return device->CreateFramebuffer(this);
    }

    void FramebufferBuilder::SetRenderPass(RenderPassBase* renderPass) {
        if ((propertiesSet & FRAMEBUFFER_PROPERTY_RENDER_PASS) != 0) {
            HandleError("Framebuffer render pass property set multiple times");
            return;
        }
        // TODO(kainino@chromium.org): null checks should not be necessary
        if (renderPass == nullptr) {
            HandleError("Render pass invalid");
            return;
        }

        this->renderPass = renderPass;
        this->textureViews.resize(renderPass->GetAttachmentCount());
        propertiesSet |= FRAMEBUFFER_PROPERTY_RENDER_PASS;
    }

    void FramebufferBuilder::SetDimensions(uint32_t width, uint32_t height) {
        if ((propertiesSet & FRAMEBUFFER_PROPERTY_DIMENSIONS) != 0) {
            HandleError("Framebuffer dimensions property set multiple times");
            return;
        }

        this->width = width;
        this->height = height;
        propertiesSet |= FRAMEBUFFER_PROPERTY_DIMENSIONS;
    }

    void FramebufferBuilder::SetAttachment(uint32_t attachmentSlot, TextureViewBase* textureView) {
        if ((propertiesSet & FRAMEBUFFER_PROPERTY_RENDER_PASS) == 0) {
            HandleError("Render pass must be set before framebuffer attachments");
            return;
        }
        if (attachmentSlot >= textureViews.size()) {
            HandleError("Attachment slot out of bounds");
            return;
        }
        if (textureViews[attachmentSlot]) {
            HandleError("Framebuffer attachment[i] set multiple times");
            return;
        }
        const auto& attachmentInfo = renderPass->GetAttachmentInfo(attachmentSlot);
        const auto* texture = textureView->GetTexture();
        if (attachmentInfo.format != texture->GetFormat()) {
            HandleError("Texture format does not match attachment format");
            return;
        }
        // TODO(kainino@chromium.org): also check attachment samples, etc.

        textureViews[attachmentSlot] = textureView;
    }
}
