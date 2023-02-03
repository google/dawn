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

#include "dawn/utils/WGPUHelpers.h"

#include <cstring>
#include <iomanip>
#include <limits>
#include <mutex>
#include <sstream>

#include "dawn/common/Constants.h"
#include "dawn/common/Log.h"
#include "dawn/common/Numeric.h"

#if TINT_BUILD_SPV_READER
#include "spirv-tools/optimizer.hpp"
#endif

namespace {
std::array<float, 12> kYuvToRGBMatrixBT709 = {1.164384f, 0.0f,       1.792741f,  -0.972945f,
                                              1.164384f, -0.213249f, -0.532909f, 0.301483f,
                                              1.164384f, 2.112402f,  0.0f,       -1.133402f};
std::array<float, 9> kGamutConversionMatrixBT709ToSrgb = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                                          0.0f, 0.0f, 0.0f, 1.0f};
std::array<float, 7> kGammaDecodeBT709 = {2.2, 1.0 / 1.099, 0.099 / 1.099, 1 / 4.5, 0.081,
                                          0.0, 0.0};
std::array<float, 7> kGammaEncodeSrgb = {1 / 2.4, 1.137119, 0.0, 12.92, 0.0031308, -0.055, 0.0};
}  // namespace

namespace utils {
#if TINT_BUILD_SPV_READER
wgpu::ShaderModule CreateShaderModuleFromASM(
    const wgpu::Device& device,
    const char* source,
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor* spirv_options) {
    // Use SPIRV-Tools's C API to assemble the SPIR-V assembly text to binary. Because the types
    // aren't RAII, we don't return directly on success and instead always go through the code
    // path that destroys the SPIRV-Tools objects.
    wgpu::ShaderModule result = nullptr;

    spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_3);
    ASSERT(context != nullptr);

    spv_binary spirv = nullptr;
    spv_diagnostic diagnostic = nullptr;
    if (spvTextToBinary(context, source, strlen(source), &spirv, &diagnostic) == SPV_SUCCESS) {
        ASSERT(spirv != nullptr);
        ASSERT(spirv->wordCount <= std::numeric_limits<uint32_t>::max());

        wgpu::ShaderModuleSPIRVDescriptor spirvDesc;
        spirvDesc.codeSize = static_cast<uint32_t>(spirv->wordCount);
        spirvDesc.code = spirv->code;
        spirvDesc.nextInChain = spirv_options;

        wgpu::ShaderModuleDescriptor descriptor;
        descriptor.nextInChain = &spirvDesc;
        result = device.CreateShaderModule(&descriptor);
    } else {
        ASSERT(diagnostic != nullptr);
        dawn::WarningLog() << "CreateShaderModuleFromASM SPIRV assembly error:"
                           << diagnostic->position.line + 1 << ":"
                           << diagnostic->position.column + 1 << ": " << diagnostic->error;
    }

    spvDiagnosticDestroy(diagnostic);
    spvBinaryDestroy(spirv);
    spvContextDestroy(context);

    return result;
}
#endif

wgpu::ShaderModule CreateShaderModule(const wgpu::Device& device, const char* source) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.source = source;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device.CreateShaderModule(&descriptor);
}

wgpu::Buffer CreateBufferFromData(const wgpu::Device& device,
                                  const void* data,
                                  uint64_t size,
                                  wgpu::BufferUsage usage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = size;
    descriptor.usage = usage | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    device.GetQueue().WriteBuffer(buffer, 0, data, size);
    return buffer;
}

ComboRenderPassDescriptor::ComboRenderPassDescriptor(
    const std::vector<wgpu::TextureView>& colorAttachmentInfo,
    wgpu::TextureView depthStencil) {
    for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
        cColorAttachments[i].loadOp = wgpu::LoadOp::Clear;
        cColorAttachments[i].storeOp = wgpu::StoreOp::Store;
        cColorAttachments[i].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
    }

    cDepthStencilAttachmentInfo.depthClearValue = 1.0f;
    cDepthStencilAttachmentInfo.stencilClearValue = 0;
    cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
    cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
    cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
    cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;

    colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfo.size());
    uint32_t colorAttachmentIndex = 0;
    for (const wgpu::TextureView& colorAttachment : colorAttachmentInfo) {
        if (colorAttachment.Get() != nullptr) {
            cColorAttachments[colorAttachmentIndex].view = colorAttachment;
        }
        ++colorAttachmentIndex;
    }
    colorAttachments = cColorAttachments.data();

    if (depthStencil.Get() != nullptr) {
        cDepthStencilAttachmentInfo.view = depthStencil;
        depthStencilAttachment = &cDepthStencilAttachmentInfo;
    } else {
        depthStencilAttachment = nullptr;
    }
}

ComboRenderPassDescriptor::~ComboRenderPassDescriptor() = default;

ComboRenderPassDescriptor::ComboRenderPassDescriptor(const ComboRenderPassDescriptor& other) {
    *this = other;
}

const ComboRenderPassDescriptor& ComboRenderPassDescriptor::operator=(
    const ComboRenderPassDescriptor& otherRenderPass) {
    cDepthStencilAttachmentInfo = otherRenderPass.cDepthStencilAttachmentInfo;
    cColorAttachments = otherRenderPass.cColorAttachments;
    colorAttachmentCount = otherRenderPass.colorAttachmentCount;

    colorAttachments = cColorAttachments.data();

    if (otherRenderPass.depthStencilAttachment != nullptr) {
        // Assign desc.depthStencilAttachment to this->depthStencilAttachmentInfo;
        depthStencilAttachment = &cDepthStencilAttachmentInfo;
    } else {
        depthStencilAttachment = nullptr;
    }

    return *this;
}
void ComboRenderPassDescriptor::UnsetDepthStencilLoadStoreOpsForFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth16Unorm:
            cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
            cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
            break;
        case wgpu::TextureFormat::Stencil8:
            cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
            cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
            break;
        default:
            break;
    }
}

BasicRenderPass::BasicRenderPass()
    : width(0),
      height(0),
      color(nullptr),
      colorFormat(wgpu::TextureFormat::RGBA8Unorm),
      renderPassInfo() {}

BasicRenderPass::BasicRenderPass(uint32_t texWidth,
                                 uint32_t texHeight,
                                 wgpu::Texture colorAttachment,
                                 wgpu::TextureFormat textureFormat)
    : width(texWidth),
      height(texHeight),
      color(colorAttachment),
      colorFormat(textureFormat),
      renderPassInfo({colorAttachment.CreateView()}) {}

BasicRenderPass CreateBasicRenderPass(const wgpu::Device& device,
                                      uint32_t width,
                                      uint32_t height,
                                      wgpu::TextureFormat format) {
    DAWN_ASSERT(width > 0 && height > 0);

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = format;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture color = device.CreateTexture(&descriptor);

    return BasicRenderPass(width, height, color);
}

wgpu::ImageCopyBuffer CreateImageCopyBuffer(wgpu::Buffer buffer,
                                            uint64_t offset,
                                            uint32_t bytesPerRow,
                                            uint32_t rowsPerImage) {
    wgpu::ImageCopyBuffer imageCopyBuffer = {};
    imageCopyBuffer.buffer = buffer;
    imageCopyBuffer.layout = CreateTextureDataLayout(offset, bytesPerRow, rowsPerImage);

    return imageCopyBuffer;
}

wgpu::ImageCopyTexture CreateImageCopyTexture(wgpu::Texture texture,
                                              uint32_t mipLevel,
                                              wgpu::Origin3D origin,
                                              wgpu::TextureAspect aspect) {
    wgpu::ImageCopyTexture imageCopyTexture;
    imageCopyTexture.texture = texture;
    imageCopyTexture.mipLevel = mipLevel;
    imageCopyTexture.origin = origin;
    imageCopyTexture.aspect = aspect;

    return imageCopyTexture;
}

wgpu::TextureDataLayout CreateTextureDataLayout(uint64_t offset,
                                                uint32_t bytesPerRow,
                                                uint32_t rowsPerImage) {
    wgpu::TextureDataLayout textureDataLayout;
    textureDataLayout.offset = offset;
    textureDataLayout.bytesPerRow = bytesPerRow;
    textureDataLayout.rowsPerImage = rowsPerImage;

    return textureDataLayout;
}

wgpu::PipelineLayout MakeBasicPipelineLayout(const wgpu::Device& device,
                                             const wgpu::BindGroupLayout* bindGroupLayout) {
    wgpu::PipelineLayoutDescriptor descriptor;
    if (bindGroupLayout != nullptr) {
        descriptor.bindGroupLayoutCount = 1;
        descriptor.bindGroupLayouts = bindGroupLayout;
    } else {
        descriptor.bindGroupLayoutCount = 0;
        descriptor.bindGroupLayouts = nullptr;
    }
    return device.CreatePipelineLayout(&descriptor);
}

wgpu::PipelineLayout MakePipelineLayout(const wgpu::Device& device,
                                        std::vector<wgpu::BindGroupLayout> bgls) {
    wgpu::PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = uint32_t(bgls.size());
    descriptor.bindGroupLayouts = bgls.data();
    return device.CreatePipelineLayout(&descriptor);
}

wgpu::BindGroupLayout MakeBindGroupLayout(
    const wgpu::Device& device,
    std::initializer_list<BindingLayoutEntryInitializationHelper> entriesInitializer) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    for (const BindingLayoutEntryInitializationHelper& entry : entriesInitializer) {
        entries.push_back(entry);
    }

    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = static_cast<uint32_t>(entries.size());
    descriptor.entries = entries.data();
    return device.CreateBindGroupLayout(&descriptor);
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

// ExternalTextureBindingLayout never contains data, so just make one that can be reused instead
// of declaring a new one every time it's needed.
wgpu::ExternalTextureBindingLayout kExternalTextureBindingLayout = {};

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::ExternalTextureBindingLayout* bindingLayout) {
    binding = entryBinding;
    visibility = entryVisibility;
    nextInChain = bindingLayout;
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    const wgpu::BindGroupLayoutEntry& entry)
    : wgpu::BindGroupLayoutEntry(entry) {}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const wgpu::Sampler& sampler)
    : binding(binding), sampler(sampler) {}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const wgpu::TextureView& textureView)
    : binding(binding), textureView(textureView) {}

BindingInitializationHelper::BindingInitializationHelper(
    uint32_t binding,
    const wgpu::ExternalTexture& externalTexture)
    : binding(binding) {
    externalTextureBindingEntry.externalTexture = externalTexture;
}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const wgpu::Buffer& buffer,
                                                         uint64_t offset,
                                                         uint64_t size)
    : binding(binding), buffer(buffer), offset(offset), size(size) {}

BindingInitializationHelper::BindingInitializationHelper(const BindingInitializationHelper&) =
    default;

BindingInitializationHelper::~BindingInitializationHelper() = default;

wgpu::BindGroupEntry BindingInitializationHelper::GetAsBinding() const {
    wgpu::BindGroupEntry result;

    result.binding = binding;
    result.sampler = sampler;
    result.textureView = textureView;
    result.buffer = buffer;
    result.offset = offset;
    result.size = size;
    if (externalTextureBindingEntry.externalTexture != nullptr) {
        result.nextInChain = &externalTextureBindingEntry;
    }

    return result;
}

wgpu::BindGroup MakeBindGroup(
    const wgpu::Device& device,
    const wgpu::BindGroupLayout& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer) {
    std::vector<wgpu::BindGroupEntry> entries;
    for (const BindingInitializationHelper& helper : entriesInitializer) {
        entries.push_back(helper.GetAsBinding());
    }

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = checked_cast<uint32_t>(entries.size());
    descriptor.entries = entries.data();

    return device.CreateBindGroup(&descriptor);
}

ColorSpaceConversionInfo GetYUVBT709ToRGBSRGBColorSpaceConversionInfo() {
    ColorSpaceConversionInfo info;
    info.yuvToRgbConversionMatrix = kYuvToRGBMatrixBT709;
    info.gamutConversionMatrix = kGamutConversionMatrixBT709ToSrgb;
    info.srcTransferFunctionParameters = kGammaDecodeBT709;
    info.dstTransferFunctionParameters = kGammaEncodeSrgb;

    return info;
}

}  // namespace utils
