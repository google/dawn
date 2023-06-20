// Copyright 2020 The Dawn Authors
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

#include "dawn/native/BindingInfo.h"

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Limits.h"

namespace dawn::native {

void IncrementBindingCounts(BindingCounts* bindingCounts, const BindGroupLayoutEntry& entry) {
    bindingCounts->totalCount += 1;

    uint32_t PerStageBindingCounts::*perStageBindingCountMember = nullptr;

    if (entry.buffer.type != wgpu::BufferBindingType::Undefined) {
        ++bindingCounts->bufferCount;
        const BufferBindingLayout& buffer = entry.buffer;

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
                UNREACHABLE();
                break;
        }
    } else if (entry.sampler.type != wgpu::SamplerBindingType::Undefined) {
        perStageBindingCountMember = &PerStageBindingCounts::samplerCount;
    } else if (entry.texture.sampleType != wgpu::TextureSampleType::Undefined) {
        perStageBindingCountMember = &PerStageBindingCounts::sampledTextureCount;
    } else if (entry.storageTexture.access != wgpu::StorageTextureAccess::Undefined) {
        perStageBindingCountMember = &PerStageBindingCounts::storageTextureCount;
    } else {
        const ExternalTextureBindingLayout* externalTextureBindingLayout;
        FindInChain(entry.nextInChain, &externalTextureBindingLayout);
        if (externalTextureBindingLayout != nullptr) {
            perStageBindingCountMember = &PerStageBindingCounts::externalTextureCount;
        }
    }

    ASSERT(perStageBindingCountMember != nullptr);
    for (SingleShaderStage stage : IterateStages(entry.visibility)) {
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

        DAWN_INVALID_IF(
            bindingCounts.perStage[stage].samplerCount > limits.v1.maxSamplersPerShaderStage,
            "The number of samplers (%u) in the %s stage exceeds the maximum per-stage limit "
            "(%u).",
            bindingCounts.perStage[stage].samplerCount, stage, limits.v1.maxSamplersPerShaderStage);

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

}  // namespace dawn::native
