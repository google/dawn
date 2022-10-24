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

#include "dawn/native/utils/WGPUHelpers.h"

#include <cstring>
#include <iomanip>
#include <limits>
#include <mutex>
#include <sstream>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/Queue.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native::utils {

ResultOrError<Ref<ShaderModuleBase>> CreateShaderModule(DeviceBase* device, const char* source) {
    ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.source = source;
    ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device->CreateShaderModule(&descriptor);
}

ResultOrError<Ref<BufferBase>> CreateBufferFromData(DeviceBase* device,
                                                    wgpu::BufferUsage usage,
                                                    const void* data,
                                                    uint64_t size) {
    BufferDescriptor descriptor;
    descriptor.size = size;
    descriptor.usage = usage;
    descriptor.mappedAtCreation = true;
    Ref<BufferBase> buffer;
    DAWN_TRY_ASSIGN(buffer, device->CreateBuffer(&descriptor));
    memcpy(buffer->GetMappedRange(0, size), data, size);
    buffer->Unmap();
    return buffer;
}

ResultOrError<Ref<PipelineLayoutBase>> MakeBasicPipelineLayout(
    DeviceBase* device,
    const Ref<BindGroupLayoutBase>& bindGroupLayout) {
    PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = 1;
    BindGroupLayoutBase* bgl = bindGroupLayout.Get();
    descriptor.bindGroupLayouts = &bgl;
    return device->CreatePipelineLayout(&descriptor);
}

ResultOrError<Ref<BindGroupLayoutBase>> MakeBindGroupLayout(
    DeviceBase* device,
    std::initializer_list<BindingLayoutEntryInitializationHelper> entriesInitializer,
    bool allowInternalBinding) {
    std::vector<BindGroupLayoutEntry> entries;
    for (const BindingLayoutEntryInitializationHelper& entry : entriesInitializer) {
        entries.push_back(entry);
    }

    BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = static_cast<uint32_t>(entries.size());
    descriptor.entries = entries.data();
    return device->CreateBindGroupLayout(&descriptor, allowInternalBinding);
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::BufferBindingType bufferType,
    bool bufferHasDynamicOffset,
    uint64_t bufferMinBindingSize) {
    binding = entryBinding;
    visibility = entryVisibility;
    buffer.type = bufferType;
    buffer.hasDynamicOffset = bufferHasDynamicOffset;
    buffer.minBindingSize = bufferMinBindingSize;
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::SamplerBindingType samplerType) {
    binding = entryBinding;
    visibility = entryVisibility;
    sampler.type = samplerType;
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::TextureSampleType textureSampleType,
    wgpu::TextureViewDimension textureViewDimension,
    bool textureMultisampled) {
    binding = entryBinding;
    visibility = entryVisibility;
    texture.sampleType = textureSampleType;
    texture.viewDimension = textureViewDimension;
    texture.multisampled = textureMultisampled;
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::StorageTextureAccess storageTextureAccess,
    wgpu::TextureFormat format,
    wgpu::TextureViewDimension textureViewDimension) {
    binding = entryBinding;
    visibility = entryVisibility;
    storageTexture.access = storageTextureAccess;
    storageTexture.format = format;
    storageTexture.viewDimension = textureViewDimension;
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    const BindGroupLayoutEntry& entry)
    : BindGroupLayoutEntry(entry) {}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const Ref<SamplerBase>& sampler)
    : binding(binding), sampler(sampler) {}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const Ref<TextureViewBase>& textureView)
    : binding(binding), textureView(textureView) {}
BindingInitializationHelper::BindingInitializationHelper(
    uint32_t binding,
    const Ref<ExternalTextureBase>& externalTexture)
    : binding(binding), externalTexture(externalTexture) {
    externalBindingEntry.externalTexture = externalTexture.Get();
}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const Ref<BufferBase>& buffer,
                                                         uint64_t offset,
                                                         uint64_t size)
    : binding(binding), buffer(buffer), offset(offset), size(size) {}

BindingInitializationHelper::~BindingInitializationHelper() = default;

BindGroupEntry BindingInitializationHelper::GetAsBinding() const {
    BindGroupEntry result;

    result.binding = binding;
    result.sampler = sampler.Get();
    result.textureView = textureView.Get();
    result.buffer = buffer.Get();
    result.offset = offset;
    result.size = size;

    if (externalTexture != nullptr) {
        result.nextInChain = &externalBindingEntry;
    }

    return result;
}

ResultOrError<Ref<BindGroupBase>> MakeBindGroup(
    DeviceBase* device,
    const Ref<BindGroupLayoutBase>& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer,
    UsageValidationMode mode) {
    std::vector<BindGroupEntry> entries;
    for (const BindingInitializationHelper& helper : entriesInitializer) {
        entries.push_back(helper.GetAsBinding());
    }

    BindGroupDescriptor descriptor;
    descriptor.layout = layout.Get();
    descriptor.entryCount = entries.size();
    descriptor.entries = entries.data();

    return device->CreateBindGroup(&descriptor, mode);
}

const char* GetLabelForTrace(const char* label) {
    return (label == nullptr || strlen(label) == 0) ? "None" : label;
}

}  // namespace dawn::native::utils
