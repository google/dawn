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

#ifndef SRC_DAWN_UTILS_WGPUHELPERS_H_
#define SRC_DAWN_UTILS_WGPUHELPERS_H_

#include <array>
#include <initializer_list>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/webgpu_cpp.h"

namespace utils {

enum Expectation { Success, Failure };

#if TINT_BUILD_SPV_READER
wgpu::ShaderModule CreateShaderModuleFromASM(
    const wgpu::Device& device,
    const char* source,
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor* spirv_options = nullptr);
#endif
wgpu::ShaderModule CreateShaderModule(const wgpu::Device& device, const char* source);

wgpu::Buffer CreateBufferFromData(const wgpu::Device& device,
                                  const void* data,
                                  uint64_t size,
                                  wgpu::BufferUsage usage);

template <typename T>
wgpu::Buffer CreateBufferFromData(const wgpu::Device& device,
                                  wgpu::BufferUsage usage,
                                  std::initializer_list<T> data) {
    return CreateBufferFromData(device, data.begin(), uint32_t(sizeof(T) * data.size()), usage);
}

wgpu::ImageCopyBuffer CreateImageCopyBuffer(wgpu::Buffer buffer,
                                            uint64_t offset = 0,
                                            uint32_t bytesPerRow = wgpu::kCopyStrideUndefined,
                                            uint32_t rowsPerImage = wgpu::kCopyStrideUndefined);
wgpu::ImageCopyTexture CreateImageCopyTexture(
    wgpu::Texture texture,
    uint32_t level = 0,
    wgpu::Origin3D origin = {0, 0, 0},
    wgpu::TextureAspect aspect = wgpu::TextureAspect::All);
wgpu::TextureDataLayout CreateTextureDataLayout(uint64_t offset,
                                                uint32_t bytesPerRow,
                                                uint32_t rowsPerImage = wgpu::kCopyStrideUndefined);

struct ComboRenderPassDescriptor : public wgpu::RenderPassDescriptor {
  public:
    ComboRenderPassDescriptor(const std::vector<wgpu::TextureView>& colorAttachmentInfo = {},
                              wgpu::TextureView depthStencil = wgpu::TextureView());
    ~ComboRenderPassDescriptor();

    ComboRenderPassDescriptor(const ComboRenderPassDescriptor& otherRenderPass);
    const ComboRenderPassDescriptor& operator=(const ComboRenderPassDescriptor& otherRenderPass);

    void UnsetDepthStencilLoadStoreOpsForFormat(wgpu::TextureFormat format);

    std::array<wgpu::RenderPassColorAttachment, kMaxColorAttachments> cColorAttachments;
    wgpu::RenderPassDepthStencilAttachment cDepthStencilAttachmentInfo = {};
};

struct BasicRenderPass {
  public:
    BasicRenderPass();
    BasicRenderPass(uint32_t width,
                    uint32_t height,
                    wgpu::Texture color,
                    wgpu::TextureFormat texture = kDefaultColorFormat);

    static constexpr wgpu::TextureFormat kDefaultColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    uint32_t width;
    uint32_t height;
    wgpu::Texture color;
    wgpu::TextureFormat colorFormat;
    utils::ComboRenderPassDescriptor renderPassInfo;
};
BasicRenderPass CreateBasicRenderPass(
    const wgpu::Device& device,
    uint32_t width,
    uint32_t height,
    wgpu::TextureFormat format = BasicRenderPass::kDefaultColorFormat);

wgpu::PipelineLayout MakeBasicPipelineLayout(const wgpu::Device& device,
                                             const wgpu::BindGroupLayout* bindGroupLayout);

wgpu::PipelineLayout MakePipelineLayout(const wgpu::Device& device,
                                        std::vector<wgpu::BindGroupLayout> bgls);

extern wgpu::ExternalTextureBindingLayout kExternalTextureBindingLayout;

// Helpers to make creating bind group layouts look nicer:
//
//   utils::MakeBindGroupLayout(device, {
//       {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
//       {1, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
//       {3, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}
//   });

struct BindingLayoutEntryInitializationHelper : wgpu::BindGroupLayoutEntry {
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
    BindingLayoutEntryInitializationHelper(uint32_t entryBinding,
                                           wgpu::ShaderStage entryVisibility,
                                           wgpu::ExternalTextureBindingLayout* bindingLayout);

    // NOLINTNEXTLINE(runtime/explicit)
    BindingLayoutEntryInitializationHelper(const wgpu::BindGroupLayoutEntry& entry);
};

wgpu::BindGroupLayout MakeBindGroupLayout(
    const wgpu::Device& device,
    std::initializer_list<BindingLayoutEntryInitializationHelper> entriesInitializer);

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
    BindingInitializationHelper(uint32_t binding, const wgpu::Sampler& sampler);
    BindingInitializationHelper(uint32_t binding, const wgpu::TextureView& textureView);
    BindingInitializationHelper(uint32_t binding, const wgpu::ExternalTexture& externalTexture);
    BindingInitializationHelper(uint32_t binding,
                                const wgpu::Buffer& buffer,
                                uint64_t offset = 0,
                                uint64_t size = wgpu::kWholeSize);
    BindingInitializationHelper(const BindingInitializationHelper&);
    ~BindingInitializationHelper();

    wgpu::BindGroupEntry GetAsBinding() const;

    uint32_t binding;
    wgpu::Sampler sampler;
    wgpu::TextureView textureView;
    wgpu::Buffer buffer;
    wgpu::ExternalTextureBindingEntry externalTextureBindingEntry;
    uint64_t offset = 0;
    uint64_t size = 0;
};

wgpu::BindGroup MakeBindGroup(
    const wgpu::Device& device,
    const wgpu::BindGroupLayout& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer);

struct ColorSpaceConversionInfo {
    std::array<float, 12> yuvToRgbConversionMatrix;
    std::array<float, 9> gamutConversionMatrix;
    std::array<float, 7> srcTransferFunctionParameters;
    std::array<float, 7> dstTransferFunctionParameters;
};

ColorSpaceConversionInfo GetYUVBT709ToRGBSRGBColorSpaceConversionInfo();

}  // namespace utils

#endif  // SRC_DAWN_UTILS_WGPUHELPERS_H_
