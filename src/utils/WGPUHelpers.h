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

#ifndef UTILS_DAWNHELPERS_H_
#define UTILS_DAWNHELPERS_H_

#include <dawn/webgpu_cpp.h>

#include <array>
#include <initializer_list>
#include <vector>

#include "common/Constants.h"
#include "utils/TextureFormatUtils.h"

namespace utils {

    enum Expectation { Success, Failure };

    enum class SingleShaderStage { Vertex, Fragment, Compute };

    wgpu::ShaderModule CreateShaderModule(const wgpu::Device& device,
                                          SingleShaderStage stage,
                                          const char* source);
    wgpu::ShaderModule CreateShaderModuleFromASM(const wgpu::Device& device, const char* source);
    wgpu::ShaderModule CreateShaderModuleFromWGSL(const wgpu::Device& device, const char* source);

    std::vector<uint32_t> CompileGLSLToSpirv(SingleShaderStage stage, const char* source);

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

    wgpu::BufferCopyView CreateBufferCopyView(wgpu::Buffer buffer,
                                              uint64_t offset,
                                              uint32_t bytesPerRow,
                                              uint32_t rowsPerImage = wgpu::kCopyStrideUndefined);
    wgpu::TextureCopyView CreateTextureCopyView(
        wgpu::Texture texture,
        uint32_t level,
        wgpu::Origin3D origin,
        wgpu::TextureAspect aspect = wgpu::TextureAspect::All);
    wgpu::TextureDataLayout CreateTextureDataLayout(
        uint64_t offset,
        uint32_t bytesPerRow,
        uint32_t rowsPerImage = wgpu::kCopyStrideUndefined);

    struct ComboRenderPassDescriptor : public wgpu::RenderPassDescriptor {
      public:
        ComboRenderPassDescriptor(std::initializer_list<wgpu::TextureView> colorAttachmentInfo,
                                  wgpu::TextureView depthStencil = wgpu::TextureView());

        ComboRenderPassDescriptor(const ComboRenderPassDescriptor& otherRenderPass);
        const ComboRenderPassDescriptor& operator=(
            const ComboRenderPassDescriptor& otherRenderPass);

        std::array<wgpu::RenderPassColorAttachmentDescriptor, kMaxColorAttachments>
            cColorAttachments;
        wgpu::RenderPassDepthStencilAttachmentDescriptor cDepthStencilAttachmentInfo = {};
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
    BasicRenderPass CreateBasicRenderPass(const wgpu::Device& device,
                                          uint32_t width,
                                          uint32_t height);

    wgpu::PipelineLayout MakeBasicPipelineLayout(const wgpu::Device& device,
                                                 const wgpu::BindGroupLayout* bindGroupLayout);

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

        // Backwards compat support for the deprecated path
        BindingLayoutEntryInitializationHelper(
            uint32_t entryBinding,
            wgpu::ShaderStage entryVisibility,
            wgpu::BindingType entryType,
            bool bufferHasDynamicOffset = false,
            uint64_t bufferMinBindingSize = 0,
            wgpu::TextureViewDimension textureViewDimension = wgpu::TextureViewDimension::Undefined,
            wgpu::TextureComponentType textureComponent = wgpu::TextureComponentType::Float,
            wgpu::TextureFormat storageFormat = wgpu::TextureFormat::Undefined);
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
        BindingInitializationHelper(uint32_t binding,
                                    const wgpu::Buffer& buffer,
                                    uint64_t offset = 0,
                                    uint64_t size = wgpu::kWholeSize);

        wgpu::BindGroupEntry GetAsBinding() const;

        uint32_t binding;
        wgpu::Sampler sampler;
        wgpu::TextureView textureView;
        wgpu::Buffer buffer;
        uint64_t offset = 0;
        uint64_t size = 0;
    };

    wgpu::BindGroup MakeBindGroup(
        const wgpu::Device& device,
        const wgpu::BindGroupLayout& layout,
        std::initializer_list<BindingInitializationHelper> entriesInitializer);

}  // namespace utils

#endif  // UTILS_DAWNHELPERS_H_
