// Copyright 2018 The NXT Authors
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

#include "backend/vulkan/FramebufferVk.h"

#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/RenderPassVk.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

    Framebuffer::Framebuffer(FramebufferBuilder* builder) : FramebufferBase(builder) {
        ASSERT(GetRenderPass()->GetAttachmentCount() <= kMaxColorAttachments + 1);

        Device* device = ToBackend(GetDevice());

        // Fill in the attachment info that will be chained in the create info.
        std::array<VkImageView, kMaxColorAttachments + 1> attachments;
        for (uint32_t i = 0; i < GetRenderPass()->GetAttachmentCount(); ++i) {
            attachments[i] = ToBackend(GetTextureView(i))->GetHandle();
        }

        // Chain attachments and create the framebuffer
        VkFramebufferCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.renderPass = ToBackend(GetRenderPass())->GetHandle();
        createInfo.attachmentCount = GetRenderPass()->GetAttachmentCount();
        createInfo.pAttachments = attachments.data();
        createInfo.width = GetWidth();
        createInfo.height = GetHeight();
        createInfo.layers = 1;

        if (device->fn.CreateFramebuffer(device->GetVkDevice(), &createInfo, nullptr, &mHandle) !=
            VK_SUCCESS) {
            ASSERT(false);
        }
    }

    Framebuffer::~Framebuffer() {
        Device* device = ToBackend(GetDevice());

        if (mHandle != VK_NULL_HANDLE) {
            device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkFramebuffer Framebuffer::GetHandle() const {
        return mHandle;
    }

    void Framebuffer::FillClearValues(VkClearValue* values) {
        const RenderPassBase* renderPass = GetRenderPass();
        for (uint32_t i = 0; i < renderPass->GetAttachmentCount(); ++i) {
            if (TextureFormatHasDepthOrStencil(renderPass->GetAttachmentInfo(i).format)) {
                const auto& clearValues = GetClearDepthStencil(i);

                values[i].depthStencil.depth = clearValues.depth;
                values[i].depthStencil.stencil = clearValues.stencil;
            } else {
                const auto& clearValues = GetClearColor(i);

                values[i].color.float32[0] = clearValues.color[0];
                values[i].color.float32[1] = clearValues.color[1];
                values[i].color.float32[2] = clearValues.color[2];
                values[i].color.float32[3] = clearValues.color[3];
            }
        }
    }

}}  // namespace backend::vulkan
