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

#include "backend/vulkan/RenderPassInfoVk.h"

#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/RenderPassCache.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/VulkanBackend.h"
#include "common/BitSetIterator.h"

namespace backend { namespace vulkan {

    RenderPassInfo::RenderPassInfo(RenderPassInfoBuilder* builder)
        : RenderPassInfoBase(builder), mDevice(ToBackend(builder->GetDevice())) {
    }

    void RenderPassInfo::RecordBeginRenderPass(VkCommandBuffer commands) {
        // Query a VkRenderPass from the cache
        VkRenderPass renderPass = VK_NULL_HANDLE;
        {
            RenderPassCacheQuery query;

            for (uint32_t i : IterateBitSet(GetColorAttachmentMask())) {
                const auto& attachmentInfo = GetColorAttachment(i);
                query.SetColor(i, attachmentInfo.view->GetTexture()->GetFormat(),
                               attachmentInfo.loadOp);
            }

            if (HasDepthStencilAttachment()) {
                const auto& attachmentInfo = GetDepthStencilAttachment();
                query.SetDepthStencil(attachmentInfo.view->GetTexture()->GetFormat(),
                                      attachmentInfo.depthLoadOp, attachmentInfo.stencilLoadOp);
            }

            renderPass = mDevice->GetRenderPassCache()->GetRenderPass(query);
        }

        // Create a framebuffer that will be used once for the render pass and gather the clear
        // values for the attachments at the same time.
        std::array<VkClearValue, kMaxColorAttachments + 1> clearValues;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        uint32_t attachmentCount = 0;
        {
            // Fill in the attachment info that will be chained in the framebuffer create info.
            std::array<VkImageView, kMaxColorAttachments + 1> attachments;

            for (uint32_t i : IterateBitSet(GetColorAttachmentMask())) {
                auto& attachmentInfo = GetColorAttachment(i);
                TextureView* view = ToBackend(attachmentInfo.view.Get());

                attachments[attachmentCount] = view->GetHandle();

                clearValues[attachmentCount].color.float32[0] = attachmentInfo.clearColor[0];
                clearValues[attachmentCount].color.float32[1] = attachmentInfo.clearColor[1];
                clearValues[attachmentCount].color.float32[2] = attachmentInfo.clearColor[2];
                clearValues[attachmentCount].color.float32[3] = attachmentInfo.clearColor[3];

                attachmentCount++;
            }

            if (HasDepthStencilAttachment()) {
                auto& attachmentInfo = GetDepthStencilAttachment();
                TextureView* view = ToBackend(attachmentInfo.view.Get());

                attachments[attachmentCount] = view->GetHandle();

                clearValues[attachmentCount].depthStencil.depth = attachmentInfo.clearDepth;
                clearValues[attachmentCount].depthStencil.stencil = attachmentInfo.clearStencil;

                attachmentCount++;
            }

            // Chain attachments and create the framebuffer
            VkFramebufferCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.renderPass = renderPass;
            createInfo.attachmentCount = attachmentCount;
            createInfo.pAttachments = attachments.data();
            createInfo.width = GetWidth();
            createInfo.height = GetHeight();
            createInfo.layers = 1;

            if (mDevice->fn.CreateFramebuffer(mDevice->GetVkDevice(), &createInfo, nullptr,
                                              &framebuffer) != VK_SUCCESS) {
                ASSERT(false);
            }

            // We don't reuse VkFramebuffers so mark the framebuffer for deletion as soon as the
            // commands currently being recorded are finished.
            mDevice->GetFencedDeleter()->DeleteWhenUnused(framebuffer);
        }

        VkRenderPassBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.renderPass = renderPass;
        beginInfo.framebuffer = framebuffer;
        beginInfo.renderArea.offset.x = 0;
        beginInfo.renderArea.offset.y = 0;
        beginInfo.renderArea.extent.width = GetWidth();
        beginInfo.renderArea.extent.height = GetHeight();
        beginInfo.clearValueCount = attachmentCount;
        beginInfo.pClearValues = clearValues.data();

        mDevice->fn.CmdBeginRenderPass(commands, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

}}  // namespace backend::vulkan
