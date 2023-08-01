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

#include "dawn/native/vulkan/BindGroupLayoutVk.h"

#include <map>
#include <utility>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/vulkan/DescriptorSetAllocator.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {

VkShaderStageFlags VulkanShaderStageFlags(wgpu::ShaderStage stages) {
    VkShaderStageFlags flags = 0;

    if (stages & wgpu::ShaderStage::Vertex) {
        flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (stages & wgpu::ShaderStage::Fragment) {
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (stages & wgpu::ShaderStage::Compute) {
        flags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }

    return flags;
}

}  // anonymous namespace

VkDescriptorType VulkanDescriptorType(const BindingInfo& bindingInfo) {
    switch (bindingInfo.bindingType) {
        case BindingInfoType::Buffer:
            switch (bindingInfo.buffer.type) {
                case wgpu::BufferBindingType::Uniform:
                    if (bindingInfo.buffer.hasDynamicOffset) {
                        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    }
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                case wgpu::BufferBindingType::Storage:
                case kInternalStorageBufferBinding:
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    if (bindingInfo.buffer.hasDynamicOffset) {
                        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    }
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                case wgpu::BufferBindingType::Undefined:
                    UNREACHABLE();
            }
        case BindingInfoType::Sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case BindingInfoType::Texture:
        case BindingInfoType::ExternalTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case BindingInfoType::StorageTexture:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    UNREACHABLE();
}

// static
ResultOrError<Ref<BindGroupLayout>> BindGroupLayout::Create(
    Device* device,
    const BindGroupLayoutDescriptor* descriptor) {
    Ref<BindGroupLayout> bgl = AcquireRef(new BindGroupLayout(device, descriptor));
    DAWN_TRY(bgl->Initialize());
    return bgl;
}

MaybeError BindGroupLayout::Initialize() {
    // Compute the bindings that will be chained in the DescriptorSetLayout create info. We add
    // one entry per binding set. This might be optimized by computing continuous ranges of
    // bindings of the same type.
    ityp::vector<BindingIndex, VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(GetBindingCount());

    for (const auto& [_, bindingIndex] : GetBindingMap()) {
        const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);

        VkDescriptorSetLayoutBinding vkBinding;
        vkBinding.binding = static_cast<uint32_t>(bindingIndex);
        vkBinding.descriptorType = VulkanDescriptorType(bindingInfo);
        vkBinding.descriptorCount = 1;
        vkBinding.stageFlags = VulkanShaderStageFlags(bindingInfo.visibility);
        vkBinding.pImmutableSamplers = nullptr;

        bindings.emplace_back(vkBinding);
    }

    VkDescriptorSetLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();

    // Record cache key information now since the createInfo is not stored.
    StreamIn(&mCacheKey, createInfo);

    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkSuccess(device->fn.CreateDescriptorSetLayout(device->GetVkDevice(), &createInfo,
                                                                 nullptr, &*mHandle),
                            "CreateDescriptorSetLayout"));

    // Compute the size of descriptor pools used for this layout.
    std::map<VkDescriptorType, uint32_t> descriptorCountPerType;

    for (BindingIndex bindingIndex{0}; bindingIndex < GetBindingCount(); ++bindingIndex) {
        VkDescriptorType vulkanType = VulkanDescriptorType(GetBindingInfo(bindingIndex));

        // map::operator[] will return 0 if the key doesn't exist.
        descriptorCountPerType[vulkanType]++;
    }

    // TODO(enga): Consider deduping allocators for layouts with the same descriptor type
    // counts.
    mDescriptorSetAllocator =
        DescriptorSetAllocator::Create(this, std::move(descriptorCountPerType));

    SetLabelImpl();

    return {};
}

BindGroupLayout::BindGroupLayout(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor)
    : BindGroupLayoutInternalBase(device, descriptor),
      mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {}

BindGroupLayout::~BindGroupLayout() = default;

void BindGroupLayout::DestroyImpl() {
    BindGroupLayoutInternalBase::DestroyImpl();

    Device* device = ToBackend(GetDevice());

    // DescriptorSetLayout aren't used by execution on the GPU and can be deleted at any time,
    // so we can destroy mHandle immediately instead of using the FencedDeleter.
    // (Swiftshader implements this wrong b/154522740).
    // In practice, the GPU is done with all descriptor sets because bind group deallocation
    // refs the bind group layout so that once the bind group is finished being used, we can
    // recycle its descriptor set.
    if (mHandle != VK_NULL_HANDLE) {
        device->fn.DestroyDescriptorSetLayout(device->GetVkDevice(), mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
    mDescriptorSetAllocator = nullptr;
}

VkDescriptorSetLayout BindGroupLayout::GetHandle() const {
    return mHandle;
}

ResultOrError<Ref<BindGroup>> BindGroupLayout::AllocateBindGroup(
    Device* device,
    const BindGroupDescriptor* descriptor) {
    DescriptorSetAllocation descriptorSetAllocation;
    DAWN_TRY_ASSIGN(descriptorSetAllocation, mDescriptorSetAllocator->Allocate());

    return AcquireRef(mBindGroupAllocator.Allocate(device, descriptor, descriptorSetAllocation));
}

void BindGroupLayout::DeallocateBindGroup(BindGroup* bindGroup,
                                          DescriptorSetAllocation* descriptorSetAllocation) {
    mDescriptorSetAllocator->Deallocate(descriptorSetAllocation);
    mBindGroupAllocator.Deallocate(bindGroup);
}

void BindGroupLayout::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_BindGroupLayout", GetLabel());
}

}  // namespace dawn::native::vulkan
