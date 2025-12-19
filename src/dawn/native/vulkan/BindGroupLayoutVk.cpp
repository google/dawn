// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/BindGroupLayoutVk.h"

#include <utility>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/vulkan/DescriptorSetAllocator.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/SamplerVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {

// Helper function (and result structure) that precomputes all the information related to static
// bindings that might be for Vulkan BindGroupLayout.
struct VulkanStaticBindings {
    ityp::vector<BindingIndex, VkDescriptorSetLayoutBinding> bindings;
    absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType;
    absl::flat_hash_map<BindingIndex, BindingIndex> textureToStaticSamplerIndex;
};
VulkanStaticBindings ComputeVulkanStaticBindings(const BindGroupLayoutInternalBase* layout) {
    VulkanStaticBindings res;

    // Build a map of texture indices to sampler indices. This maps the texture to
    // the sampler which will be used in VK in the combined image sampler entry.
    for (BindingIndex bindingIndex : layout->GetStaticSamplerIndices()) {
        auto samplerBindingInfo =
            std::get<StaticSamplerBindingInfo>(layout->GetBindingInfo(bindingIndex).bindingLayout);
        if (!samplerBindingInfo.isUsedForSingleTexture) {
            // The client did not specify that this sampler should be paired
            // with a single texture binding.
            continue;
        }

        res.textureToStaticSamplerIndex[samplerBindingInfo.sampledTextureIndex] = bindingIndex;
    }

    // Compute the bindings that will be chained in the DescriptorSetLayout create info. We add
    // one entry per binding set. This might be optimized by computing continuous ranges of
    // bindings of the same type.
    res.bindings.reserve(layout->GetBindingCount());

    for (BindingIndex bindingIndex : Range(layout->GetBindingCount())) {
        // This texture will be bound into the VkDescriptorSet at the index for the sampler itself.
        if (res.textureToStaticSamplerIndex.contains(bindingIndex)) {
            continue;
        }

        // Vulkan descriptor set layouts have one entry for binding_array. Only handle their first
        // element as subsequent ones will be part of the already added
        // VkDescriptorSetLayoutBinding.
        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);
        if (bindingInfo.indexInArray != BindingIndex(0)) {
            continue;
        }

        VkDescriptorSetLayoutBinding vkBinding{
            .binding = uint32_t(bindingIndex),
            .descriptorType = VulkanDescriptorType(bindingInfo),
            .descriptorCount = uint32_t(bindingInfo.arraySize),
            .stageFlags = VulkanShaderStages(bindingInfo.visibility),
            .pImmutableSamplers = nullptr,
        };
        size_t descriptorCount = vkBinding.descriptorCount;

        // Static samplers are set at VkDescriptorSetLayout creation time.
        if (std::holds_alternative<StaticSamplerBindingInfo>(bindingInfo.bindingLayout)) {
            auto samplerLayout = std::get<StaticSamplerBindingInfo>(bindingInfo.bindingLayout);
            auto sampler = ToBackend(samplerLayout.sampler);
            vkBinding.pImmutableSamplers = &sampler->GetHandle().GetHandle();

            if (sampler->IsYCbCr()) {
                // A YCbCr sampler can take up multiple Vk descriptor slots.  There is a
                // recommended Vulkan API to query how many slots a YCbCr sampler should take, but
                // it is not clear how to actually pass the Android external format to that API.
                // However, the spec for that API says the following:
                // "combinedImageSamplerDescriptorCount is a number between 1 and the number of
                // planes in the format. A descriptor set layout binding with immutable Y′CBCR
                // conversion samplers will have a maximum combinedImageSamplerDescriptorCount
                // which is the maximum across all formats supported by its samplers of the
                // combinedImageSamplerDescriptorCount for each format." Hence, we simply hardcode
                // the maximum number of planes that an external format can have here. The number
                // of overall YCbCr descriptors will be relatively small and these pools are not an
                // overall bottleneck on memory usage.
                DAWN_ASSERT(bindingInfo.arraySize == BindingIndex(1));
                descriptorCount = 3;
            }
        }

        res.bindings.emplace_back(vkBinding);

        // absl:flat_hash_map::operator[] will return 0 if the key doesn't exist.
        res.descriptorCountPerType[vkBinding.descriptorType] += descriptorCount;
    }

    return res;
}

}  // anonymous namespace

VkDescriptorType VulkanDescriptorType(const BindingInfo& bindingInfo) {
    return MatchVariant(
        bindingInfo.bindingLayout,
        [](const BufferBindingInfo& layout) -> VkDescriptorType {
            switch (layout.type) {
                case wgpu::BufferBindingType::Uniform:
                    if (layout.hasDynamicOffset) {
                        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    }
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                case wgpu::BufferBindingType::Storage:
                case kInternalStorageBufferBinding:
                case wgpu::BufferBindingType::ReadOnlyStorage:
                case kInternalReadOnlyStorageBufferBinding:
                    if (layout.hasDynamicOffset) {
                        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    }
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                case wgpu::BufferBindingType::BindingNotUsed:
                case wgpu::BufferBindingType::Undefined:
                    DAWN_UNREACHABLE();
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            }
            DAWN_UNREACHABLE();
        },
        [](const SamplerBindingInfo&) { return VK_DESCRIPTOR_TYPE_SAMPLER; },
        [](const StaticSamplerBindingInfo& layout) {
            // Make this entry into a combined image sampler iff the client
            // specified a single texture binding to be paired with it.
            return (layout.isUsedForSingleTexture) ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                                                   : VK_DESCRIPTOR_TYPE_SAMPLER;
        },
        [](const TextureBindingInfo&) { return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; },
        [](const StorageTextureBindingInfo&) { return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; },
        [](const TexelBufferBindingInfo&) {
            // TODO(crbug/382544164): Prototype texel buffer feature
            DAWN_UNREACHABLE();
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        },

        [](const InputAttachmentBindingInfo&) { return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; },
        [](const ExternalTextureBindingInfo&) -> VkDescriptorType { DAWN_UNREACHABLE(); });
}

// static
ResultOrError<Ref<BindGroupLayout>> BindGroupLayout::Create(
    Device* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor) {
    Ref<BindGroupLayoutStaticBindingOnly> bgl =
        AcquireRef(new BindGroupLayoutStaticBindingOnly(device, descriptor));
    DAWN_TRY(bgl->Initialize());
    return bgl;
}

BindGroupLayout::BindGroupLayout(DeviceBase* device,
                                 const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor)
    : BindGroupLayoutInternalBase(device, descriptor),
      mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {}

BindGroupLayout::~BindGroupLayout() = default;

MaybeError BindGroupLayout::Initialize(
    const VkDescriptorSetLayoutCreateInfo* createInfo,
    absl::flat_hash_map<BindingIndex, BindingIndex> textureToStaticSamplerIndex) {
    // Record cache key information now since the createInfo is not stored.
    StreamIn(&mCacheKey, createInfo);

    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateDescriptorSetLayout(device->GetVkDevice(), createInfo, nullptr, &*mHandle),
        "CreateDescriptorSetLayout"));

    mTextureToStaticSamplerIndex = std::move(textureToStaticSamplerIndex);

    SetLabelImpl();

    return {};
}

void BindGroupLayout::DestroyImpl() {
    BindGroupLayoutInternalBase::DestroyImpl();

    Device* device = ToBackend(GetDevice());

    // DescriptorSetLayout aren't used by execution on the GPU and can be deleted at any time,
    // so we can destroy mHandle immediately instead of using the FencedDeleter.
    if (mHandle != VK_NULL_HANDLE) {
        device->fn.DestroyDescriptorSetLayout(device->GetVkDevice(), mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

VkDescriptorSetLayout BindGroupLayout::GetHandle() const {
    return mHandle;
}

void BindGroupLayout::DeallocateBindGroup(BindGroup* bindGroup) {
    mBindGroupAllocator->Deallocate(bindGroup);
}


void BindGroupLayout::ReduceMemoryUsage() {
    mBindGroupAllocator->DeleteEmptySlabs();
}

std::optional<BindingIndex> BindGroupLayout::GetStaticSamplerIndexForTexture(
    BindingIndex textureBinding) const {
    if (mTextureToStaticSamplerIndex.contains(textureBinding)) {
        return mTextureToStaticSamplerIndex.at(textureBinding);
    }
    return {};
}

void BindGroupLayout::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_BindGroupLayout", GetLabel());
}

// BindGroupLayoutStaticBindingOnly

BindGroupLayoutStaticBindingOnly::BindGroupLayoutStaticBindingOnly(
    DeviceBase* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor)
    : BindGroupLayout(device, descriptor) {}

BindGroupLayoutStaticBindingOnly::~BindGroupLayoutStaticBindingOnly() = default;

MaybeError BindGroupLayoutStaticBindingOnly::Initialize() {
    VulkanStaticBindings bindings = ComputeVulkanStaticBindings(this);

    mDescriptorSetAllocator = DescriptorSetAllocator::Create(
        ToBackend(GetDevice()), std::move(bindings.descriptorCountPerType));

    VkDescriptorSetLayoutCreateInfo createInfo{
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        createInfo.pNext = nullptr,
        createInfo.flags = 0,
        createInfo.bindingCount = uint32_t(bindings.bindings.size()),
        createInfo.pBindings = bindings.bindings.data(),
    };

    return BindGroupLayout::Initialize(&createInfo,
                                       std::move(bindings.textureToStaticSamplerIndex));
}

ResultOrError<Ref<BindGroup>> BindGroupLayoutStaticBindingOnly::AllocateBindGroup(
    const UnpackedPtr<BindGroupDescriptor>& descriptor) {
    Device* device = ToBackend(GetDevice());

    DescriptorSetAllocation descriptorSetAllocation;
    DAWN_TRY_ASSIGN(descriptorSetAllocation, mDescriptorSetAllocator->Allocate(GetHandle()));

    return AcquireRef(mBindGroupAllocator->Allocate(device, descriptor, descriptorSetAllocation));
}

void BindGroupLayoutStaticBindingOnly::DeallocateDescriptorSet(
    DescriptorSetAllocation* descriptorSetAllocation) {
    mDescriptorSetAllocator->Deallocate(descriptorSetAllocation);
}

void BindGroupLayoutStaticBindingOnly::DestroyImpl() {
    BindGroupLayout::DestroyImpl();

    mDescriptorSetAllocator = nullptr;
}

}  // namespace dawn::native::vulkan
