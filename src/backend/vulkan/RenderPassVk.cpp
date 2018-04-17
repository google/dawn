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

#include "backend/vulkan/RenderPassVk.h"

#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

    namespace {
        VkAttachmentLoadOp VulkanAttachmentLoadOp(nxt::LoadOp op) {
            switch (op) {
                case nxt::LoadOp::Load:
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
                case nxt::LoadOp::Clear:
                    return VK_ATTACHMENT_LOAD_OP_CLEAR;
                default:
                    UNREACHABLE();
            }
        }
    }  // anonymous namespace

    RenderPass::RenderPass(RenderPassBuilder* builder)
        : RenderPassBase(builder), mDevice(ToBackend(builder->GetDevice())) {
        // For now we only support single pass render passes.
        ASSERT(GetSubpassCount() == 1);
        ASSERT(GetAttachmentCount() <= kMaxColorAttachments + 1);

        const auto& subpass = GetSubpassInfo(0);

        // The Vulkan subpasses want to know the layout of the attachments with VkAttachmentRef.
        // Precompute them as they must be pointer-chained in VkSubpassDescription
        std::array<VkAttachmentReference, kMaxColorAttachments + 1> attachmentRefs;
        attachmentRefs.fill(VkAttachmentReference{VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED});

        for (uint32_t i : IterateBitSet(subpass.colorAttachmentsSet)) {
            attachmentRefs[i].attachment = subpass.colorAttachments[i];
            attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            // TODO(cwallez@chromium.org): need validation rule that attachments are packed
            ASSERT(i == 0 || subpass.colorAttachmentsSet[i - 1]);
        }
        if (subpass.depthStencilAttachment) {
            attachmentRefs[kMaxColorAttachments].attachment = subpass.depthStencilAttachment;
            attachmentRefs[kMaxColorAttachments].layout =
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        // Create the VkSubpassDescription that will be chained in the VkRenderPassCreateInfo
        VkSubpassDescription subpassDesc;
        subpassDesc.flags = 0;
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.inputAttachmentCount = 0;
        subpassDesc.pInputAttachments = nullptr;
        subpassDesc.colorAttachmentCount =
            static_cast<uint32_t>(subpass.colorAttachmentsSet.count());
        subpassDesc.pColorAttachments = attachmentRefs.data();
        subpassDesc.pResolveAttachments = nullptr;
        subpassDesc.pDepthStencilAttachment = &attachmentRefs[kMaxColorAttachments];
        subpassDesc.preserveAttachmentCount = 0;
        subpassDesc.pPreserveAttachments = nullptr;

        // Create the VkAttachmentDescriptions that will be chained in the VkRenderPassCreateInfo
        std::array<VkAttachmentDescription, kMaxColorAttachments + 1> attachmentDescs = {};
        for (uint32_t i = 0; i < GetAttachmentCount(); ++i) {
            const auto& attachment = GetAttachmentInfo(i);
            auto& attachmentDesc = attachmentDescs[i];

            attachmentDesc.flags = 0;
            attachmentDesc.format = VulkanImageFormat(attachment.format);
            attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            if (TextureFormatHasDepthOrStencil(attachment.format)) {
                attachmentDesc.loadOp = VulkanAttachmentLoadOp(attachment.depthLoadOp);
                attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachmentDesc.stencilLoadOp = VulkanAttachmentLoadOp(attachment.stencilLoadOp);
                attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

                attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            } else {
                attachmentDesc.loadOp = VulkanAttachmentLoadOp(attachment.colorLoadOp);
                attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

                attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }

        // Chain everything in VkRenderPassCreateInfo
        VkRenderPassCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.attachmentCount = GetAttachmentCount();
        createInfo.pAttachments = attachmentDescs.data();
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpassDesc;
        createInfo.dependencyCount = 0;
        createInfo.pDependencies = nullptr;

        // Create the render pass from the zillion parameters
        if (mDevice->fn.CreateRenderPass(mDevice->GetVkDevice(), &createInfo, nullptr, &mHandle) !=
            VK_SUCCESS) {
            ASSERT(false);
        }
    }

    RenderPass::~RenderPass() {
        if (mHandle != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkRenderPass RenderPass::GetHandle() const {
        return mHandle;
    }

}}  // namespace backend::vulkan
