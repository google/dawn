// Copyright 2018 The Dawn Authors
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

#include "dawn/native/vulkan/RenderPassCache.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/HashUtils.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {
VkAttachmentLoadOp VulkanAttachmentLoadOp(wgpu::LoadOp op) {
    switch (op) {
        case wgpu::LoadOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case wgpu::LoadOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case wgpu::LoadOp::Undefined:
            UNREACHABLE();
            break;
    }
    UNREACHABLE();
}

VkAttachmentStoreOp VulkanAttachmentStoreOp(wgpu::StoreOp op) {
    // TODO(crbug.com/dawn/485): return STORE_OP_STORE_NONE_QCOM if the device has required
    // extension.
    switch (op) {
        case wgpu::StoreOp::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case wgpu::StoreOp::Discard:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case wgpu::StoreOp::Undefined:
            UNREACHABLE();
            break;
    }
    UNREACHABLE();
}
}  // anonymous namespace

// RenderPassCacheQuery

void RenderPassCacheQuery::SetColor(ColorAttachmentIndex index,
                                    wgpu::TextureFormat format,
                                    wgpu::LoadOp loadOp,
                                    wgpu::StoreOp storeOp,
                                    bool hasResolveTarget) {
    colorMask.set(index);
    colorFormats[index] = format;
    colorLoadOp[index] = loadOp;
    colorStoreOp[index] = storeOp;
    resolveTargetMask[index] = hasResolveTarget;
}

void RenderPassCacheQuery::SetDepthStencil(wgpu::TextureFormat format,
                                           wgpu::LoadOp depthLoadOpIn,
                                           wgpu::StoreOp depthStoreOpIn,
                                           wgpu::LoadOp stencilLoadOpIn,
                                           wgpu::StoreOp stencilStoreOpIn,
                                           bool readOnly) {
    hasDepthStencil = true;
    depthStencilFormat = format;
    depthLoadOp = depthLoadOpIn;
    depthStoreOp = depthStoreOpIn;
    stencilLoadOp = stencilLoadOpIn;
    stencilStoreOp = stencilStoreOpIn;
    readOnlyDepthStencil = readOnly;
}

void RenderPassCacheQuery::SetSampleCount(uint32_t sampleCount) {
    this->sampleCount = sampleCount;
}

// RenderPassCache

RenderPassCache::RenderPassCache(Device* device) : mDevice(device) {}

RenderPassCache::~RenderPassCache() {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto [_, renderPass] : mCache) {
        mDevice->fn.DestroyRenderPass(mDevice->GetVkDevice(), renderPass, nullptr);
    }

    mCache.clear();
}

ResultOrError<VkRenderPass> RenderPassCache::GetRenderPass(const RenderPassCacheQuery& query) {
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mCache.find(query);
    if (it != mCache.end()) {
        return VkRenderPass(it->second);
    }

    VkRenderPass renderPass;
    DAWN_TRY_ASSIGN(renderPass, CreateRenderPassForQuery(query));
    mCache.emplace(query, renderPass);
    return renderPass;
}

ResultOrError<VkRenderPass> RenderPassCache::CreateRenderPassForQuery(
    const RenderPassCacheQuery& query) const {
    // The Vulkan subpasses want to know the layout of the attachments with VkAttachmentRef.
    // Precompute them as they must be pointer-chained in VkSubpassDescription.
    // Note that both colorAttachmentRefs and resolveAttachmentRefs can be sparse with holes
    // filled with VK_ATTACHMENT_UNUSED.
    ityp::array<ColorAttachmentIndex, VkAttachmentReference, kMaxColorAttachments>
        colorAttachmentRefs;
    ityp::array<ColorAttachmentIndex, VkAttachmentReference, kMaxColorAttachments>
        resolveAttachmentRefs;
    VkAttachmentReference depthStencilAttachmentRef;

    for (ColorAttachmentIndex i(uint8_t(0)); i < kMaxColorAttachmentsTyped; i++) {
        colorAttachmentRefs[i].attachment = VK_ATTACHMENT_UNUSED;
        resolveAttachmentRefs[i].attachment = VK_ATTACHMENT_UNUSED;
        // The Khronos Vulkan validation layer will complain if not set
        colorAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        resolveAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    // Contains the attachment description that will be chained in the create info
    // The order of all attachments in attachmentDescs is "color-depthstencil-resolve".
    constexpr uint8_t kMaxAttachmentCount = kMaxColorAttachments * 2 + 1;
    std::array<VkAttachmentDescription, kMaxAttachmentCount> attachmentDescs = {};

    VkSampleCountFlagBits vkSampleCount = VulkanSampleCount(query.sampleCount);

    uint32_t attachmentCount = 0;
    ColorAttachmentIndex highestColorAttachmentIndexPlusOne(static_cast<uint8_t>(0));
    for (ColorAttachmentIndex i : IterateBitSet(query.colorMask)) {
        auto& attachmentRef = colorAttachmentRefs[i];
        auto& attachmentDesc = attachmentDescs[attachmentCount];

        attachmentRef.attachment = attachmentCount;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentDesc.flags = 0;
        attachmentDesc.format = VulkanImageFormat(mDevice, query.colorFormats[i]);
        attachmentDesc.samples = vkSampleCount;
        attachmentDesc.loadOp = VulkanAttachmentLoadOp(query.colorLoadOp[i]);
        attachmentDesc.storeOp = VulkanAttachmentStoreOp(query.colorStoreOp[i]);
        attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentCount++;
        highestColorAttachmentIndexPlusOne =
            ColorAttachmentIndex(static_cast<uint8_t>(static_cast<uint8_t>(i) + 1u));
    }

    VkAttachmentReference* depthStencilAttachment = nullptr;
    if (query.hasDepthStencil) {
        auto& attachmentDesc = attachmentDescs[attachmentCount];

        depthStencilAttachment = &depthStencilAttachmentRef;

        depthStencilAttachmentRef.attachment = attachmentCount;
        depthStencilAttachmentRef.layout = query.readOnlyDepthStencil
                                               ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                               : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachmentDesc.flags = 0;
        attachmentDesc.format = VulkanImageFormat(mDevice, query.depthStencilFormat);
        attachmentDesc.samples = vkSampleCount;

        attachmentDesc.loadOp = VulkanAttachmentLoadOp(query.depthLoadOp);
        attachmentDesc.storeOp = VulkanAttachmentStoreOp(query.depthStoreOp);
        attachmentDesc.stencilLoadOp = VulkanAttachmentLoadOp(query.stencilLoadOp);
        attachmentDesc.stencilStoreOp = VulkanAttachmentStoreOp(query.stencilStoreOp);

        // There is only one subpass, so it is safe to set both initialLayout and finalLayout to
        // the only subpass's layout.
        attachmentDesc.initialLayout = depthStencilAttachmentRef.layout;
        attachmentDesc.finalLayout = depthStencilAttachmentRef.layout;

        attachmentCount++;
    }

    uint32_t resolveAttachmentCount = 0;
    for (ColorAttachmentIndex i : IterateBitSet(query.resolveTargetMask)) {
        auto& attachmentRef = resolveAttachmentRefs[i];
        auto& attachmentDesc = attachmentDescs[attachmentCount];

        attachmentRef.attachment = attachmentCount;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentDesc.flags = 0;
        attachmentDesc.format = VulkanImageFormat(mDevice, query.colorFormats[i]);
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentCount++;
        resolveAttachmentCount++;
    }

    // Create the VkSubpassDescription that will be chained in the VkRenderPassCreateInfo
    VkSubpassDescription subpassDesc;
    subpassDesc.flags = 0;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.colorAttachmentCount = static_cast<uint8_t>(highestColorAttachmentIndexPlusOne);
    subpassDesc.pColorAttachments = colorAttachmentRefs.data();

    // Qualcomm GPUs have a driver bug on some devices where passing a zero-length array to the
    // resolveAttachments causes a VK_ERROR_OUT_OF_HOST_MEMORY. nullptr must be passed instead.
    if (resolveAttachmentCount) {
        subpassDesc.pResolveAttachments = resolveAttachmentRefs.data();
    } else {
        subpassDesc.pResolveAttachments = nullptr;
    }

    subpassDesc.pDepthStencilAttachment = depthStencilAttachment;
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = nullptr;

    // Chain everything in VkRenderPassCreateInfo
    VkRenderPassCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = attachmentCount;
    createInfo.pAttachments = attachmentDescs.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpassDesc;
    createInfo.dependencyCount = 0;
    createInfo.pDependencies = nullptr;

    // Create the render pass from the zillion parameters
    VkRenderPass renderPass;
    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.CreateRenderPass(mDevice->GetVkDevice(), &createInfo, nullptr, &*renderPass),
        "CreateRenderPass"));
    return renderPass;
}

// RenderPassCache

size_t RenderPassCache::CacheFuncs::operator()(const RenderPassCacheQuery& query) const {
    size_t hash = Hash(query.colorMask);

    HashCombine(&hash, Hash(query.resolveTargetMask));

    for (ColorAttachmentIndex i : IterateBitSet(query.colorMask)) {
        HashCombine(&hash, query.colorFormats[i], query.colorLoadOp[i], query.colorStoreOp[i]);
    }

    HashCombine(&hash, query.hasDepthStencil);
    if (query.hasDepthStencil) {
        HashCombine(&hash, query.depthStencilFormat, query.depthLoadOp, query.depthStoreOp,
                    query.stencilLoadOp, query.stencilStoreOp, query.readOnlyDepthStencil);
    }

    HashCombine(&hash, query.sampleCount);

    return hash;
}

bool RenderPassCache::CacheFuncs::operator()(const RenderPassCacheQuery& a,
                                             const RenderPassCacheQuery& b) const {
    if (a.colorMask != b.colorMask) {
        return false;
    }

    if (a.resolveTargetMask != b.resolveTargetMask) {
        return false;
    }

    if (a.sampleCount != b.sampleCount) {
        return false;
    }

    for (ColorAttachmentIndex i : IterateBitSet(a.colorMask)) {
        if ((a.colorFormats[i] != b.colorFormats[i]) || (a.colorLoadOp[i] != b.colorLoadOp[i]) ||
            (a.colorStoreOp[i] != b.colorStoreOp[i])) {
            return false;
        }
    }

    if (a.hasDepthStencil != b.hasDepthStencil) {
        return false;
    }

    if (a.hasDepthStencil) {
        if ((a.depthStencilFormat != b.depthStencilFormat) || (a.depthLoadOp != b.depthLoadOp) ||
            (a.stencilLoadOp != b.stencilLoadOp) || (a.depthStoreOp != b.depthStoreOp) ||
            (a.stencilStoreOp != b.stencilStoreOp) ||
            (a.readOnlyDepthStencil != b.readOnlyDepthStencil)) {
            return false;
        }
    }

    return true;
}
}  // namespace dawn::native::vulkan
