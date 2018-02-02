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

#include "backend/vulkan/BindGroupLayoutVk.h"

#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

    namespace {

        VkDescriptorType VulkanDescriptorType(nxt::BindingType type) {
            switch (type) {
                case nxt::BindingType::UniformBuffer:
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                case nxt::BindingType::Sampler:
                    return VK_DESCRIPTOR_TYPE_SAMPLER;
                case nxt::BindingType::SampledTexture:
                    return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                case nxt::BindingType::StorageBuffer:
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                default:
                    UNREACHABLE();
            }
        }

        VkShaderStageFlags VulkanShaderStageFlags(nxt::ShaderStageBit stages) {
            VkShaderStageFlags flags = 0;

            if (stages & nxt::ShaderStageBit::Vertex) {
                flags |= VK_SHADER_STAGE_VERTEX_BIT;
            }
            if (stages & nxt::ShaderStageBit::Fragment) {
                flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            if (stages & nxt::ShaderStageBit::Compute) {
                flags |= VK_SHADER_STAGE_COMPUTE_BIT;
            }

            return flags;
        }

    }  // anonymous namespace

    BindGroupLayout::BindGroupLayout(BindGroupLayoutBuilder* builder)
        : BindGroupLayoutBase(builder) {
        const auto& info = GetBindingInfo();

        // Compute the bindings that will be chained in the DescriptorSetLayout create info. We add
        // one entry per binding set. This might be optimized by computing continuous ranges of
        // bindings of the same type.
        uint32_t numBindings = 0;
        std::array<VkDescriptorSetLayoutBinding, kMaxBindingsPerGroup> bindings;
        for (uint32_t bindingIndex : IterateBitSet(info.mask)) {
            auto& binding = bindings[numBindings];
            binding.binding = bindingIndex;
            binding.descriptorType = VulkanDescriptorType(info.types[bindingIndex]);
            binding.descriptorCount = 1;
            binding.stageFlags = VulkanShaderStageFlags(info.visibilities[bindingIndex]);
            binding.pImmutableSamplers = nullptr;

            numBindings++;
        }

        VkDescriptorSetLayoutCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.bindingCount = numBindings;
        createInfo.pBindings = bindings.data();

        Device* device = ToBackend(GetDevice());
        if (device->fn.CreateDescriptorSetLayout(device->GetVkDevice(), &createInfo, nullptr,
                                                 &mHandle) != VK_SUCCESS) {
            ASSERT(false);
        }
    }

    BindGroupLayout::~BindGroupLayout() {
        // DescriptorSetLayout aren't used by execution on the GPU and can be deleted at any time,
        // so we destroy mHandle immediately instead of using the FencedDeleter
        if (mHandle != VK_NULL_HANDLE) {
            Device* device = ToBackend(GetDevice());
            device->fn.DestroyDescriptorSetLayout(device->GetVkDevice(), mHandle, nullptr);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkDescriptorSetLayout BindGroupLayout::GetHandle() const {
        return mHandle;
    }

}}  // namespace backend::vulkan
