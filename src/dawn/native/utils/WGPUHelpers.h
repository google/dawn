// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_UTILS_WGPUHELPERS_H_
#define SRC_DAWN_NATIVE_UTILS_WGPUHELPERS_H_

#include <array>
#include <initializer_list>
#include <vector>

#include "dawn/common/RefCounted.h"
#include "dawn/native/Error.h"
#include "dawn/native/UsageValidationMode.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native::utils {

ResultOrError<Ref<ShaderModuleBase>> CreateShaderModule(DeviceBase* device, const char* source);

ResultOrError<Ref<BufferBase>> CreateBufferFromData(DeviceBase* device,
                                                    wgpu::BufferUsage usage,
                                                    const void* data,
                                                    uint64_t size);

template <typename T>
ResultOrError<Ref<BufferBase>> CreateBufferFromData(DeviceBase* device,
                                                    wgpu::BufferUsage usage,
                                                    std::initializer_list<T> data) {
    return CreateBufferFromData(device, usage, data.begin(), uint32_t(sizeof(T) * data.size()));
}

ResultOrError<Ref<PipelineLayoutBase>> MakeBasicPipelineLayout(
    DeviceBase* device,
    const Ref<BindGroupLayoutBase>& bindGroupLayout);

// Helpers to make creating bind group layouts look nicer:
//
//   utils::MakeBindGroupLayout(device, {
//       {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
//       {1, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
//       {3, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}
//   });

struct BindingLayoutEntryInitializationHelper : BindGroupLayoutEntry {
    BindingLayoutEntryInitializationHelper(uint32_t entryBinding,
                                           wgpu::ShaderStage entryVisibility,
                                           wgpu::BufferBindingType bufferType,
                                           bool bufferHasDynamicOffset = false,
                                           uint64_t bufferMinBindingSize = 0);
    BindingLayoutEntryInitializationHelper(uint32_t entryBinding,
                                           wgpu::ShaderStage entryVisibility,
                                           wgpu::SamplerBindingType samplerType);
    BindingLayoutEntryInitializationHelper(
        uint32_t entryBinding,
        wgpu::ShaderStage entryVisibility,
        wgpu::TextureSampleType textureSampleType,
        wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D,
        bool textureMultisampled = false);
    BindingLayoutEntryInitializationHelper(
        uint32_t entryBinding,
        wgpu::ShaderStage entryVisibility,
        wgpu::StorageTextureAccess storageTextureAccess,
        wgpu::TextureFormat format,
        wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D);

    explicit BindingLayoutEntryInitializationHelper(const BindGroupLayoutEntry& entry);
};

ResultOrError<Ref<BindGroupLayoutBase>> MakeBindGroupLayout(
    DeviceBase* device,
    std::initializer_list<BindingLayoutEntryInitializationHelper> entriesInitializer,
    bool allowInternalBinding = false);

// Helpers to make creating bind groups look nicer:
//
//   utils::MakeBindGroup(device, layout, {
//       {0, mySampler},
//       {1, myBuffer, offset, size},
//       {3, myTextureView}
//   });

// Structure with one constructor per-type of bindings, so that the initializer_list accepts
// bindings with the right type and no extra information.
struct BindingInitializationHelper {
    BindingInitializationHelper(uint32_t binding, const Ref<SamplerBase>& sampler);
    BindingInitializationHelper(uint32_t binding, const Ref<TextureViewBase>& textureView);
    BindingInitializationHelper(uint32_t binding, const Ref<ExternalTextureBase>& externalTexture);
    BindingInitializationHelper(uint32_t binding,
                                const Ref<BufferBase>& buffer,
                                uint64_t offset = 0,
                                uint64_t size = wgpu::kWholeSize);
    ~BindingInitializationHelper();

    BindGroupEntry GetAsBinding() const;

    uint32_t binding;
    Ref<SamplerBase> sampler;
    Ref<TextureViewBase> textureView;
    Ref<BufferBase> buffer;
    Ref<ExternalTextureBase> externalTexture;
    ExternalTextureBindingEntry externalBindingEntry;
    uint64_t offset = 0;
    uint64_t size = 0;
};

// This helper is only used inside dawn native.
ResultOrError<Ref<BindGroupBase>> MakeBindGroup(
    DeviceBase* device,
    const Ref<BindGroupLayoutBase>& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer,
    UsageValidationMode mode);

const char* GetLabelForTrace(const char* label);

}  // namespace dawn::native::utils

#endif  // SRC_DAWN_NATIVE_UTILS_WGPUHELPERS_H_
