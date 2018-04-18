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

#include "backend/vulkan/SamplerVk.h"

#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

    namespace {
        VkSamplerAddressMode VulkanSamplerAddressMode(nxt::AddressMode mode) {
            switch (mode) {
                case nxt::AddressMode::Repeat:
                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
                case nxt::AddressMode::MirroredRepeat:
                    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                case nxt::AddressMode::ClampToEdge:
                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                default:
                    UNREACHABLE();
            }
        }

        VkFilter VulkanSamplerFilter(nxt::FilterMode filter) {
            switch (filter) {
                case nxt::FilterMode::Linear:
                    return VK_FILTER_LINEAR;
                case nxt::FilterMode::Nearest:
                    return VK_FILTER_NEAREST;
                default:
                    UNREACHABLE();
            }
        }

        VkSamplerMipmapMode VulkanMipMapMode(nxt::FilterMode filter) {
            switch (filter) {
                case nxt::FilterMode::Linear:
                    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
                case nxt::FilterMode::Nearest:
                    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
                default:
                    UNREACHABLE();
            }
        }
    }  // anonymous namespace

    Sampler::Sampler(SamplerBuilder* builder)
        : SamplerBase(builder), mDevice(ToBackend(builder->GetDevice())) {
        VkSamplerCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.magFilter = VulkanSamplerFilter(builder->GetMagFilter());
        createInfo.minFilter = VulkanSamplerFilter(builder->GetMinFilter());
        createInfo.mipmapMode = VulkanMipMapMode(builder->GetMipMapFilter());
        createInfo.addressModeU = VulkanSamplerAddressMode(builder->GetAddressModeU());
        createInfo.addressModeV = VulkanSamplerAddressMode(builder->GetAddressModeV());
        createInfo.addressModeW = VulkanSamplerAddressMode(builder->GetAddressModeW());
        createInfo.mipLodBias = 0.0f;
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.maxAnisotropy = 1.0f;
        createInfo.compareEnable = VK_FALSE;
        createInfo.compareOp = VK_COMPARE_OP_NEVER;
        createInfo.minLod = 0.0f;
        createInfo.maxLod = 1000.0f;
        createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        createInfo.unnormalizedCoordinates = VK_FALSE;

        if (mDevice->fn.CreateSampler(mDevice->GetVkDevice(), &createInfo, nullptr, &mHandle) !=
            VK_SUCCESS) {
            ASSERT(false);
        }
    }

    Sampler::~Sampler() {
        if (mHandle != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkSampler Sampler::GetHandle() const {
        return mHandle;
    }

}}  // namespace backend::vulkan
