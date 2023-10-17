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
                DAWN_UNREACHABLE();
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

    DAWN_ASSERT(perStageBindingCountMember != nullptr);
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
