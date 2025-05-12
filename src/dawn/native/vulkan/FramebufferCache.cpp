// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/FramebufferCache.h"

#include "absl/container/inlined_vector.h"
#include "dawn/common/HashUtils.h"
#include "dawn/common/Range.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// FramebufferCacheQuery

void FramebufferCacheQuery::SetRenderPass(VkRenderPass pass,
                                          uint32_t passWidth,
                                          uint32_t passHeight) {
    renderPass = pass;
    width = passWidth;
    height = passHeight;
}

uint32_t FramebufferCacheQuery::AddAttachment(VkImageView attachment) {
    attachments[attachmentCount] = attachment;
    return attachmentCount++;
}

// FramebufferCache

FramebufferCache::FramebufferCache(Device* device, size_t capacity)
    : mDevice(device), mCapacity(capacity) {}

FramebufferCache::~FramebufferCache() {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto [_, framebuffer] : mRecentList) {
        mDevice->fn.DestroyFramebuffer(mDevice->GetVkDevice(), framebuffer, nullptr);
    }

    mRecentList.clear();
    mCache.clear();
}

bool FramebufferCache::IsCacheDisabled() const {
    return mDevice->IsToggleEnabled(Toggle::VulkanDisableFramebufferCache);
}

ResultOrError<VkFramebuffer> FramebufferCache::GetFramebuffer(FramebufferCacheQuery& query) {
    if (IsCacheDisabled()) {
        // Some devices (such as older Qualcomm GPUs) appear to have problems with reusing
        // framebuffers, so don't attempt to cache if it's been disabled.
        VkFramebuffer framebuffer;
        DAWN_TRY_ASSIGN(framebuffer, CreateFramebufferForQuery(query));
        mDevice->GetFencedDeleter()->DeleteWhenUnused(framebuffer);
        return framebuffer;
    }

    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mCache.find(query);
    if (it != mCache.end()) {
        // Move the queried framebuffer to the front of the LRU list.
        // Using the iterator as a stable reference like this works because "Adding, removing and
        // moving the elements within the list or across several lists does not invalidate the
        // iterators or references. An iterator is invalidated only when the corresponding element
        // is deleted." (From https://en.cppreference.com/w/cpp/container/list)
        mRecentList.splice(mRecentList.begin(), mRecentList, it->second);
        return it->second->second;
    }

    VkFramebuffer framebuffer;
    DAWN_TRY_ASSIGN(framebuffer, CreateFramebufferForQuery(query));

    mRecentList.emplace_front(query, framebuffer);
    mCache.emplace(query, mRecentList.begin());

    if (mRecentList.size() > mCapacity) {
        auto back = mRecentList.back();
        mDevice->GetFencedDeleter()->DeleteWhenUnused(back.second);
        mCache.erase(back.first);
        mRecentList.pop_back();
    }

    return framebuffer;
}

ResultOrError<VkFramebuffer> FramebufferCache::CreateFramebufferForQuery(
    FramebufferCacheQuery& query) const {
    VkFramebufferCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.renderPass = query.renderPass;
    createInfo.attachmentCount = query.attachmentCount;
    createInfo.pAttachments = AsVkArray(query.attachments.data());
    createInfo.width = query.width;
    createInfo.height = query.height;
    createInfo.layers = 1;

    VkFramebuffer framebuffer;
    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.CreateFramebuffer(mDevice->GetVkDevice(), &createInfo, nullptr, &*framebuffer),
        "CreateFramebuffer"));
    return framebuffer;
}

size_t FramebufferCache::CacheFuncs::operator()(const FramebufferCacheQuery& query) const {
    size_t hash = Hash(query.renderPass.GetHandle());

    HashCombine(&hash, query.width, query.height, query.attachmentCount);

    for (uint32_t i = 0; i < query.attachmentCount; ++i) {
        HashCombine(&hash, query.attachments[i].GetHandle());
    }

    return hash;
}

bool FramebufferCache::CacheFuncs::operator()(const FramebufferCacheQuery& a,
                                              const FramebufferCacheQuery& b) const {
    if (a.renderPass != b.renderPass || a.width != b.width || a.height != b.height ||
        a.attachmentCount != b.attachmentCount) {
        return false;
    }

    for (uint32_t i = 0; i < a.attachmentCount; ++i) {
        if (a.attachments[i] != b.attachments[i]) {
            return false;
        }
    }

    return true;
}

}  // namespace dawn::native::vulkan
