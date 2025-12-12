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

#include "dawn/native/vulkan/ResourceTableVk.h"

#include "dawn/native/DynamicArrayState.h"
#include "dawn/native/vulkan/DescriptorSetAllocator.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
ResultOrError<Ref<ResourceTable>> ResourceTable::Create(Device* device,
                                                        const ResourceTableDescriptor* descriptor) {
    Ref<ResourceTable> table = AcquireRef(new ResourceTable(device, descriptor));
    DAWN_TRY(table->Initialize());
    return table;
}

// static
ResultOrError<VkDescriptorSetLayout> ResourceTable::MakeDescriptorSetLayout(Device* device) {
    // A resource table is a bindgroup made of two entries:
    //
    //  - Binding 0: the metadata storage buffer.
    //  - Binding 1: the variable length, partially bound, update-after-bind array of sampled
    //  textures/samplers.
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
        {{
             .binding = 0,
             .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
             .descriptorCount = 1,
             .stageFlags = VulkanShaderStages(kAllStages),
             .pImmutableSamplers = nullptr,
         },
         {
             .binding = 1,
             .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
             .descriptorCount = device->GetLimits().resourceTableLimits.maxResourceTableSize +
                                kReservedDynamicBindingArrayEntries,
             .stageFlags = VulkanShaderStages(kAllStages),
             .pImmutableSamplers = nullptr,
         }}};
    std::array<VkDescriptorBindingFlags, 2> flags = {
        0,  //
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = flags.size(),
        .pBindingFlags = flags.data(),
    };
    VkDescriptorSetLayoutCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flagCreateInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = bindings.size(),
        .pBindings = bindings.data(),
    };

    VkDescriptorSetLayout dsLayout = VK_NULL_HANDLE;
    DAWN_TRY(CheckVkSuccess(device->fn.CreateDescriptorSetLayout(device->GetVkDevice(), &createInfo,
                                                                 nullptr, &*dsLayout),
                            "CreateDescriptorSetLayout"));
    return dsLayout;
}

ResourceTable::~ResourceTable() = default;

MaybeError ResourceTable::Initialize() {
    DAWN_TRY(ResourceTableBase::InitializeBase());

    Device* device = ToBackend(GetDevice());

    // Allocate the VkDescriptorSet used for this resource table.
    mDSAllocator = DescriptorSetAllocatorDynamicArray::Create(device);

    // The only non-dynamic binding (using the terminology of DescriptorSetAllocatorDynamicArray) is
    // the metadata buffer.
    absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType;
    descriptorCountPerType[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = 1;

    DAWN_TRY_ASSIGN(
        mAllocation,
        mDSAllocator->Allocate(device->GetResourceTableLayout(), descriptorCountPerType,
                               VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                               uint32_t(GetDynamicArrayState()->GetSizeWithDefaultBindings())));

    // Only write the metadata buffer in the VkDescriptorSet initially, all the other bindings will
    // be written as needed when they are inserted in the ResourceTable.
    Buffer* metadataBuffer = ToBackend(GetDynamicArrayState()->GetMetadataBuffer());
    VkDescriptorBufferInfo bufferInfo = {
        .buffer = metadataBuffer->GetHandle(),
        .offset = 0,
        .range = metadataBuffer->GetSize(),
    };
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = mAllocation.set,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo,
        .pTexelBufferView = nullptr,
    };
    device->fn.UpdateDescriptorSets(device->GetVkDevice(), 1, &write, 0, nullptr);

    return {};
}

void ResourceTable::DestroyImpl() {
    mDSAllocator->Deallocate(&mAllocation);
    mDSAllocator = nullptr;

    ResourceTableBase::DestroyImpl();
}

void ResourceTable::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mAllocation.set, "Dawn_ResourceTable", GetLabel());
}

}  // namespace dawn::native::vulkan
