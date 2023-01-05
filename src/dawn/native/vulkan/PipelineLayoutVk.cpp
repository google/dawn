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

#include "dawn/native/vulkan/PipelineLayoutVk.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
ResultOrError<Ref<PipelineLayout>> PipelineLayout::Create(
    Device* device,
    const PipelineLayoutDescriptor* descriptor) {
    Ref<PipelineLayout> layout = AcquireRef(new PipelineLayout(device, descriptor));
    DAWN_TRY(layout->Initialize());
    return layout;
}

MaybeError PipelineLayout::Initialize() {
    // Compute the array of VkDescriptorSetLayouts that will be chained in the create info.
    // TODO(crbug.com/dawn/277) Vulkan doesn't allow holes in this array, should we expose
    // this constraints at the Dawn level?
    uint32_t numSetLayouts = 0;
    std::array<VkDescriptorSetLayout, kMaxBindGroups> setLayouts;
    std::array<const CachedObject*, kMaxBindGroups> cachedObjects;
    for (BindGroupIndex setIndex : IterateBitSet(GetBindGroupLayoutsMask())) {
        const BindGroupLayoutBase* bindGroupLayout = GetBindGroupLayout(setIndex);
        setLayouts[numSetLayouts] = ToBackend(bindGroupLayout)->GetHandle();
        cachedObjects[numSetLayouts] = bindGroupLayout;
        numSetLayouts++;
    }

    // Always reserve push constant space for the ClampFragDepthArgs.
    VkPushConstantRange depthClampArgsRange;
    depthClampArgsRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    depthClampArgsRange.offset = kClampFragDepthArgsOffset;
    depthClampArgsRange.size = kClampFragDepthArgsSize;

    VkPipelineLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.setLayoutCount = numSetLayouts;
    createInfo.pSetLayouts = AsVkArray(setLayouts.data());
    createInfo.pushConstantRangeCount = 1;
    createInfo.pPushConstantRanges = &depthClampArgsRange;

    // Record cache key information now since the createInfo is not stored.
    StreamIn(&mCacheKey, stream::Iterable(cachedObjects.data(), numSetLayouts), createInfo);

    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkSuccess(
        device->fn.CreatePipelineLayout(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "CreatePipelineLayout"));

    SetLabelImpl();

    return {};
}

PipelineLayout::~PipelineLayout() = default;

void PipelineLayout::DestroyImpl() {
    PipelineLayoutBase::DestroyImpl();
    if (mHandle != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        mHandle = VK_NULL_HANDLE;
    }
}

VkPipelineLayout PipelineLayout::GetHandle() const {
    return mHandle;
}

void PipelineLayout::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_PipelineLayout", GetLabel());
}

}  // namespace dawn::native::vulkan
