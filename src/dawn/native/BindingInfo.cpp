// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/native/BindingInfo.h"

#include "dawn/common/MatchVariant.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Limits.h"
#include "dawn/native/Sampler.h"

namespace dawn::native {

BindingInfoType GetBindingInfoType(const BindingInfo& info) {
    return MatchVariant(
        info.bindingLayout,
        [](const BufferBindingInfo&) -> BindingInfoType { return BindingInfoType::Buffer; },
        [](const SamplerBindingInfo&) -> BindingInfoType { return BindingInfoType::Sampler; },
        [](const TextureBindingInfo&) -> BindingInfoType { return BindingInfoType::Texture; },
        [](const StorageTextureBindingInfo&) -> BindingInfoType {
            return BindingInfoType::StorageTexture;
        },
        [](const StaticSamplerBindingInfo&) -> BindingInfoType {
            return BindingInfoType::StaticSampler;
        },
        [](const InputAttachmentBindingInfo&) -> BindingInfoType {
            return BindingInfoType::InputAttachment;
        });
}

void IncrementBindingCounts(BindingCounts* bindingCounts,
                            const UnpackedPtr<BindGroupLayoutEntry>& entry) {
    bindingCounts->totalCount += 1;

    uint32_t PerStageBindingCounts::*perStageBindingCountMember = nullptr;

    if (entry->buffer.type != wgpu::BufferBindingType::Undefined) {
        ++bindingCounts->bufferCount;
        const BufferBindingLayout& buffer = entry->buffer;

        if (buffer.minBindingSize == 0) {
            ++bindingCounts->unverifiedBufferCount;
        }

        switch (buffer.type) {
            case wgpu::BufferBindingType::Uniform:
                if (buffer.hasDynamicOffset) {
                    ++bindingCounts->dynamicUniformBufferCount;
                }
                perStageBindingCountMember = &PerStageBindingCounts::uniformBufferCount;
                break;

            case wgpu::BufferBindingType::Storage:
            case kInternalStorageBufferBinding:
            case wgpu::BufferBindingType::ReadOnlyStorage:
                if (buffer.hasDynamicOffset) {
                    ++bindingCounts->dynamicStorageBufferCount;
                }
                perStageBindingCountMember = &PerStageBindingCounts::storageBufferCount;
                break;

            case wgpu::BufferBindingType::Undefined:
                // Can't get here due to the enclosing if statement.
                DAWN_UNREACHABLE();
                break;
        }
    } else if (entry->sampler.type != wgpu::SamplerBindingType::Undefined) {
        perStageBindingCountMember = &PerStageBindingCounts::samplerCount;
    } else if (entry->texture.sampleType != wgpu::TextureSampleType::Undefined) {
        if (entry->texture.viewDimension == kInternalInputAttachmentDim) {
            // Internal use only.
            return;
        } else {
            perStageBindingCountMember = &PerStageBindingCounts::sampledTextureCount;
        }
    } else if (entry->storageTexture.access != wgpu::StorageTextureAccess::Undefined) {
        perStageBindingCountMember = &PerStageBindingCounts::storageTextureCount;
    } else if (entry.Get<ExternalTextureBindingLayout>()) {
        perStageBindingCountMember = &PerStageBindingCounts::externalTextureCount;
    } else if (entry.Get<StaticSamplerBindingLayout>()) {
        ++bindingCounts->staticSamplerCount;
        perStageBindingCountMember = &PerStageBindingCounts::staticSamplerCount;
    }

    DAWN_ASSERT(perStageBindingCountMember != nullptr);
    for (SingleShaderStage stage : IterateStages(entry->visibility)) {
        ++(bindingCounts->perStage[stage].*perStageBindingCountMember);
    }
}

void AccumulateBindingCounts(BindingCounts* bindingCounts, const BindingCounts& rhs) {
    bindingCounts->totalCount += rhs.totalCount;
    bindingCounts->bufferCount += rhs.bufferCount;
    bindingCounts->unverifiedBufferCount += rhs.unverifiedBufferCount;
    bindingCounts->dynamicUniformBufferCount += rhs.dynamicUniformBufferCount;
    bindingCounts->dynamicStorageBufferCount += rhs.dynamicStorageBufferCount;

    for (SingleShaderStage stage : IterateStages(kAllStages)) {
        bindingCounts->perStage[stage].sampledTextureCount +=
            rhs.perStage[stage].sampledTextureCount;
        bindingCounts->perStage[stage].samplerCount += rhs.perStage[stage].samplerCount;
        bindingCounts->perStage[stage].storageBufferCount += rhs.perStage[stage].storageBufferCount;
        bindingCounts->perStage[stage].storageTextureCount +=
            rhs.perStage[stage].storageTextureCount;
        bindingCounts->perStage[stage].uniformBufferCount += rhs.perStage[stage].uniformBufferCount;
        bindingCounts->perStage[stage].externalTextureCount +=
            rhs.perStage[stage].externalTextureCount;
        bindingCounts->perStage[stage].staticSamplerCount += rhs.perStage[stage].staticSamplerCount;
    }
}

MaybeError ValidateBindingCounts(const CombinedLimits& limits, const BindingCounts& bindingCounts) {
    DAWN_INVALID_IF(
        bindingCounts.dynamicUniformBufferCount >
            limits.v1.maxDynamicUniformBuffersPerPipelineLayout,
        "The number of dynamic uniform buffers (%u) exceeds the maximum per-pipeline-layout "
        "limit (%u).",
        bindingCounts.dynamicUniformBufferCount,
        limits.v1.maxDynamicUniformBuffersPerPipelineLayout);

    DAWN_INVALID_IF(
        bindingCounts.dynamicStorageBufferCount >
            limits.v1.maxDynamicStorageBuffersPerPipelineLayout,
        "The number of dynamic storage buffers (%u) exceeds the maximum per-pipeline-layout "
        "limit (%u).",
        bindingCounts.dynamicStorageBufferCount,
        limits.v1.maxDynamicStorageBuffersPerPipelineLayout);

    for (SingleShaderStage stage : IterateStages(kAllStages)) {
        DAWN_INVALID_IF(bindingCounts.perStage[stage].sampledTextureCount >
                            limits.v1.maxSampledTexturesPerShaderStage,
                        "The number of sampled textures (%u) in the %s stage exceeds the maximum "
                        "per-stage limit (%u).",
                        bindingCounts.perStage[stage].sampledTextureCount, stage,
                        limits.v1.maxSampledTexturesPerShaderStage);

        // The per-stage number of external textures is bound by the maximum sampled textures
        // per stage.
        DAWN_INVALID_IF(
            bindingCounts.perStage[stage].externalTextureCount >
                limits.v1.maxSampledTexturesPerShaderStage / kSampledTexturesPerExternalTexture,
            "The number of external textures (%u) in the %s stage exceeds the maximum "
            "per-stage limit (%u).",
            bindingCounts.perStage[stage].externalTextureCount, stage,
            limits.v1.maxSampledTexturesPerShaderStage / kSampledTexturesPerExternalTexture);

        DAWN_INVALID_IF(
            bindingCounts.perStage[stage].sampledTextureCount +
                    (bindingCounts.perStage[stage].externalTextureCount *
                     kSampledTexturesPerExternalTexture) >
                limits.v1.maxSampledTexturesPerShaderStage,
            "The combination of sampled textures (%u) and external textures (%u) in the %s "
            "stage exceeds the maximum per-stage limit (%u).",
            bindingCounts.perStage[stage].sampledTextureCount,
            bindingCounts.perStage[stage].externalTextureCount, stage,
            limits.v1.maxSampledTexturesPerShaderStage);

        // TODO(crbug.com/dawn/2463): Account for static samplers here.
        DAWN_INVALID_IF(
            bindingCounts.perStage[stage].samplerCount > limits.v1.maxSamplersPerShaderStage,
            "The number of samplers (%u) in the %s stage exceeds the maximum per-stage limit "
            "(%u).",
            bindingCounts.perStage[stage].samplerCount, stage, limits.v1.maxSamplersPerShaderStage);

        // TODO(crbug.com/dawn/2463): Account for static samplers here.
        DAWN_INVALID_IF(
            bindingCounts.perStage[stage].samplerCount +
                    (bindingCounts.perStage[stage].externalTextureCount *
                     kSamplersPerExternalTexture) >
                limits.v1.maxSamplersPerShaderStage,
            "The combination of samplers (%u) and external textures (%u) in the %s stage "
            "exceeds the maximum per-stage limit (%u).",
            bindingCounts.perStage[stage].samplerCount,
            bindingCounts.perStage[stage].externalTextureCount, stage,
            limits.v1.maxSamplersPerShaderStage);

        DAWN_INVALID_IF(
            bindingCounts.perStage[stage].storageBufferCount >
                limits.v1.maxStorageBuffersPerShaderStage,
            "The number of storage buffers (%u) in the %s stage exceeds the maximum per-stage "
            "limit (%u).",
            bindingCounts.perStage[stage].storageBufferCount, stage,
            limits.v1.maxStorageBuffersPerShaderStage);

        DAWN_INVALID_IF(
            bindingCounts.perStage[stage].storageTextureCount >
                limits.v1.maxStorageTexturesPerShaderStage,
            "The number of storage textures (%u) in the %s stage exceeds the maximum per-stage "
            "limit (%u).",
            bindingCounts.perStage[stage].storageTextureCount, stage,
            limits.v1.maxStorageTexturesPerShaderStage);

        DAWN_INVALID_IF(
            bindingCounts.perStage[stage].uniformBufferCount >
                limits.v1.maxUniformBuffersPerShaderStage,
            "The number of uniform buffers (%u) in the %s stage exceeds the maximum per-stage "
            "limit (%u).",
            bindingCounts.perStage[stage].uniformBufferCount, stage,
            limits.v1.maxUniformBuffersPerShaderStage);

        DAWN_INVALID_IF(
            bindingCounts.perStage[stage].uniformBufferCount +
                    (bindingCounts.perStage[stage].externalTextureCount *
                     kUniformsPerExternalTexture) >
                limits.v1.maxUniformBuffersPerShaderStage,
            "The combination of uniform buffers (%u) and external textures (%u) in the %s "
            "stage exceeds the maximum per-stage limit (%u).",
            bindingCounts.perStage[stage].uniformBufferCount,
            bindingCounts.perStage[stage].externalTextureCount, stage,
            limits.v1.maxUniformBuffersPerShaderStage);
    }

    return {};
}

BufferBindingInfo::BufferBindingInfo() = default;

BufferBindingInfo::BufferBindingInfo(const BufferBindingLayout& apiLayout)
    : type(apiLayout.type),
      minBindingSize(apiLayout.minBindingSize),
      hasDynamicOffset(apiLayout.hasDynamicOffset) {}

TextureBindingInfo::TextureBindingInfo() {}

TextureBindingInfo::TextureBindingInfo(const TextureBindingLayout& apiLayout)
    : sampleType(apiLayout.sampleType),
      viewDimension(apiLayout.viewDimension),
      multisampled(apiLayout.multisampled) {}

StorageTextureBindingInfo::StorageTextureBindingInfo() = default;

StorageTextureBindingInfo::StorageTextureBindingInfo(const StorageTextureBindingLayout& apiLayout)
    : format(apiLayout.format), viewDimension(apiLayout.viewDimension), access(apiLayout.access) {}

SamplerBindingInfo::SamplerBindingInfo() = default;

SamplerBindingInfo::SamplerBindingInfo(const SamplerBindingLayout& apiLayout)
    : type(apiLayout.type) {}

StaticSamplerBindingInfo::StaticSamplerBindingInfo(const StaticSamplerBindingLayout& apiLayout)
    : sampler(apiLayout.sampler),
      sampledTextureBinding(BindingNumber{apiLayout.sampledTextureBinding}),
      isUsedForSingleTextureBinding(apiLayout.sampledTextureBinding < WGPU_LIMIT_U32_UNDEFINED) {}

InputAttachmentBindingInfo::InputAttachmentBindingInfo() = default;
InputAttachmentBindingInfo::InputAttachmentBindingInfo(wgpu::TextureSampleType sampleType)
    : sampleType(sampleType) {}

}  // namespace dawn::native
