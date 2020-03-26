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

#include "dawn_native/vulkan/BindGroupLayoutVk.h"

#include "common/BitSetIterator.h"
#include "dawn_native/vulkan/BindGroupVk.h"
#include "dawn_native/vulkan/DescriptorSetService.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/FencedDeleter.h"
#include "dawn_native/vulkan/VulkanError.h"

#include <map>

namespace dawn_native { namespace vulkan {

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

    VkDescriptorType VulkanDescriptorType(wgpu::BindingType type, bool isDynamic) {
        switch (type) {
            case wgpu::BindingType::UniformBuffer:
                if (isDynamic) {
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                }
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case wgpu::BindingType::Sampler:
                return VK_DESCRIPTOR_TYPE_SAMPLER;
            case wgpu::BindingType::SampledTexture:
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case wgpu::BindingType::StorageBuffer:
            case wgpu::BindingType::ReadonlyStorageBuffer:
                if (isDynamic) {
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                }
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case wgpu::BindingType::ReadonlyStorageTexture:
            case wgpu::BindingType::WriteonlyStorageTexture:
                return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case wgpu::BindingType::StorageTexture:
            default:
                UNREACHABLE();
        }
    }

    // static
    ResultOrError<BindGroupLayout*> BindGroupLayout::Create(
        Device* device,
        const BindGroupLayoutDescriptor* descriptor) {
        std::unique_ptr<BindGroupLayout> bgl =
            std::make_unique<BindGroupLayout>(device, descriptor);
        DAWN_TRY(bgl->Initialize());
        return bgl.release();
    }

    MaybeError BindGroupLayout::Initialize() {
        // Compute the bindings that will be chained in the DescriptorSetLayout create info. We add
        // one entry per binding set. This might be optimized by computing continuous ranges of
        // bindings of the same type.
        uint32_t numBindings = 0;
        std::array<VkDescriptorSetLayoutBinding, kMaxBindingsPerGroup> bindings;
        for (const auto& it : GetBindingMap()) {
            BindingNumber bindingNumber = it.first;
            BindingIndex bindingIndex = it.second;
            const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);

            VkDescriptorSetLayoutBinding* vkBinding = &bindings[numBindings];
            vkBinding->binding = bindingNumber;
            vkBinding->descriptorType =
                VulkanDescriptorType(bindingInfo.type, bindingInfo.hasDynamicOffset);
            vkBinding->descriptorCount = 1;
            vkBinding->stageFlags = VulkanShaderStageFlags(bindingInfo.visibility);
            vkBinding->pImmutableSamplers = nullptr;

            numBindings++;
        }

        VkDescriptorSetLayoutCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.bindingCount = numBindings;
        createInfo.pBindings = bindings.data();

        Device* device = ToBackend(GetDevice());
        DAWN_TRY(CheckVkSuccess(device->fn.CreateDescriptorSetLayout(
                                    device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
                                "CreateDescriptorSetLayout"));

        // Compute the size of descriptor pools used for this layout.
        std::map<VkDescriptorType, uint32_t> descriptorCountPerType;

        for (BindingIndex bindingIndex = 0; bindingIndex < GetBindingCount(); ++bindingIndex) {
            const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);
            VkDescriptorType vulkanType =
                VulkanDescriptorType(bindingInfo.type, bindingInfo.hasDynamicOffset);

            // map::operator[] will return 0 if the key doesn't exist.
            descriptorCountPerType[vulkanType]++;
        }

        mPoolSizes.reserve(descriptorCountPerType.size());
        for (const auto& it : descriptorCountPerType) {
            mPoolSizes.push_back(VkDescriptorPoolSize{it.first, it.second});
        }

        return {};
    }

    BindGroupLayout::BindGroupLayout(DeviceBase* device,
                                     const BindGroupLayoutDescriptor* descriptor)
        : BindGroupLayoutBase(device, descriptor),
          mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {
    }

    BindGroupLayout::~BindGroupLayout() {
        Device* device = ToBackend(GetDevice());

        // DescriptorSetLayout aren't used by execution on the GPU and can be deleted at any time,
        // so we destroy mHandle immediately instead of using the FencedDeleter
        if (mHandle != VK_NULL_HANDLE) {
            device->fn.DestroyDescriptorSetLayout(device->GetVkDevice(), mHandle, nullptr);
            mHandle = VK_NULL_HANDLE;
        }

        FencedDeleter* deleter = device->GetFencedDeleter();
        for (const SingleDescriptorSetAllocation& allocation : mAllocations) {
            deleter->DeleteWhenUnused(allocation.pool);
        }
        mAllocations.clear();
    }

    VkDescriptorSetLayout BindGroupLayout::GetHandle() const {
        return mHandle;
    }

    ResultOrError<BindGroup*> BindGroupLayout::AllocateBindGroup(
        Device* device,
        const BindGroupDescriptor* descriptor) {
        DescriptorSetAllocation descriptorSetAllocation;
        DAWN_TRY_ASSIGN(descriptorSetAllocation, AllocateOneDescriptorSet());
        return mBindGroupAllocator.Allocate(device, descriptor, descriptorSetAllocation);
    }

    void BindGroupLayout::DeallocateBindGroup(BindGroup* bindGroup) {
        mBindGroupAllocator.Deallocate(bindGroup);
    }

    ResultOrError<DescriptorSetAllocation> BindGroupLayout::AllocateOneDescriptorSet() {
        Device* device = ToBackend(GetDevice());

        // Reuse a previous allocation if available.
        if (!mAvailableAllocations.empty()) {
            size_t index = mAvailableAllocations.back();
            mAvailableAllocations.pop_back();
            return {{index, mAllocations[index].set}};
        }

        // Create a pool to hold our descriptor set.
        // TODO(cwallez@chromium.org): This horribly inefficient, have more than one descriptor
        // set per pool.
        VkDescriptorPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.maxSets = 1;
        createInfo.poolSizeCount = static_cast<uint32_t>(mPoolSizes.size());
        createInfo.pPoolSizes = mPoolSizes.data();

        VkDescriptorPool descriptorPool;
        DAWN_TRY(CheckVkSuccess(device->fn.CreateDescriptorPool(device->GetVkDevice(), &createInfo,
                                                                nullptr, &*descriptorPool),
                                "CreateDescriptorPool"));

        // Allocate our single set.
        VkDescriptorSetAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.descriptorPool = descriptorPool;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &*mHandle;

        VkDescriptorSet descriptorSet;
        MaybeError result =
            CheckVkSuccess(device->fn.AllocateDescriptorSets(device->GetVkDevice(), &allocateInfo,
                                                             &*descriptorSet),
                           "AllocateDescriptorSets");

        if (result.IsError()) {
            // On an error we can destroy the pool immediately because no command references it.
            device->fn.DestroyDescriptorPool(device->GetVkDevice(), descriptorPool, nullptr);
            return result.AcquireError();
        }

        mAllocations.push_back({descriptorPool, descriptorSet});
        return {{mAllocations.size() - 1, descriptorSet}};
    }

    void BindGroupLayout::DeallocateDescriptorSet(
        DescriptorSetAllocation* descriptorSetAllocation) {
        // We can't reuse the descriptor set right away because the Vulkan spec says in the
        // documentation for vkCmdBindDescriptorSets that the set may be consumed any time between
        // host execution of the command and the end of the draw/dispatch.
        ToBackend(GetDevice())
            ->GetDescriptorSetService()
            ->AddDeferredDeallocation(this, descriptorSetAllocation->index);

        // Clear the content of allocation so that use after frees are more visible.
        *descriptorSetAllocation = {};
    }

    void BindGroupLayout::FinishDeallocation(size_t index) {
        mAvailableAllocations.push_back(index);
    }

}}  // namespace dawn_native::vulkan
