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

#include "dawn_native/BindingInfo.h"

#include "dawn_native/ChainUtils_autogen.h"

namespace dawn_native {

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
            bindingCounts->perStage[stage].storageBufferCount +=
                rhs.perStage[stage].storageBufferCount;
            bindingCounts->perStage[stage].storageTextureCount +=
                rhs.perStage[stage].storageTextureCount;
            bindingCounts->perStage[stage].uniformBufferCount +=
                rhs.perStage[stage].uniformBufferCount;
            bindingCounts->perStage[stage].externalTextureCount +=
                rhs.perStage[stage].externalTextureCount;
        }
    }

    MaybeError ValidateBindingCounts(const BindingCounts& bindingCounts) {
        if (bindingCounts.dynamicUniformBufferCount > kMaxDynamicUniformBuffersPerPipelineLayout) {
            return DAWN_VALIDATION_ERROR(
                "The number of dynamic uniform buffers exceeds the maximum per-pipeline-layout "
                "limit");
        }

        if (bindingCounts.dynamicStorageBufferCount > kMaxDynamicStorageBuffersPerPipelineLayout) {
            return DAWN_VALIDATION_ERROR(
                "The number of dynamic storage buffers exceeds the maximum per-pipeline-layout "
                "limit");
        }

        for (SingleShaderStage stage : IterateStages(kAllStages)) {
            if (bindingCounts.perStage[stage].sampledTextureCount >
                kMaxSampledTexturesPerShaderStage) {
                return DAWN_VALIDATION_ERROR(
                    "The number of sampled textures exceeds the maximum "
                    "per-stage limit.");
            }

            // The per-stage number of external textures is bound by the maximum sampled textures
            // per stage.
            if (bindingCounts.perStage[stage].externalTextureCount >
                kMaxSampledTexturesPerShaderStage / kSampledTexturesPerExternalTexture) {
                return DAWN_VALIDATION_ERROR(
                    "The number of external textures exceeds the maximum "
                    "per-stage limit.");
            }

            if (bindingCounts.perStage[stage].sampledTextureCount +
                    (bindingCounts.perStage[stage].externalTextureCount *
                     kSampledTexturesPerExternalTexture) >
                kMaxSampledTexturesPerShaderStage) {
                return DAWN_VALIDATION_ERROR(
                    "The combination of sampled textures and external textures exceeds the maximum "
                    "per-stage limit.");
            }

            if (bindingCounts.perStage[stage].samplerCount > kMaxSamplersPerShaderStage) {
                return DAWN_VALIDATION_ERROR(
                    "The number of samplers exceeds the maximum per-stage limit.");
            }

            if (bindingCounts.perStage[stage].samplerCount +
                    (bindingCounts.perStage[stage].externalTextureCount *
                     kSamplersPerExternalTexture) >
                kMaxSamplersPerShaderStage) {
                return DAWN_VALIDATION_ERROR(
                    "The combination of samplers and external textures exceeds the maximum "
                    "per-stage limit.");
            }

            if (bindingCounts.perStage[stage].storageBufferCount >
                kMaxStorageBuffersPerShaderStage) {
                return DAWN_VALIDATION_ERROR(
                    "The number of storage buffers exceeds the maximum per-stage limit.");
            }

            if (bindingCounts.perStage[stage].storageTextureCount >
                kMaxStorageTexturesPerShaderStage) {
                return DAWN_VALIDATION_ERROR(
                    "The number of storage textures exceeds the maximum per-stage limit.");
            }

            if (bindingCounts.perStage[stage].uniformBufferCount >
                kMaxUniformBuffersPerShaderStage) {
                return DAWN_VALIDATION_ERROR(
                    "The number of uniform buffers exceeds the maximum per-stage limit.");
            }

            if (bindingCounts.perStage[stage].uniformBufferCount +
                    (bindingCounts.perStage[stage].externalTextureCount *
                     kUniformsPerExternalTexture) >
                kMaxUniformBuffersPerShaderStage) {
                return DAWN_VALIDATION_ERROR(
                    "The combination of uniform buffers and external textures exceeds the maximum "
                    "per-stage limit.");
            }
        }

        return {};
    }

}  // namespace dawn_native
