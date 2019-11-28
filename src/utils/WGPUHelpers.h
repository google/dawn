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

#include "common/Constants.h"

namespace utils {

    enum Expectation { Success, Failure };

    enum class SingleShaderStage { Vertex, Fragment, Compute };

    wgpu::ShaderModule CreateShaderModule(const wgpu::Device& device,
                                          SingleShaderStage stage,
                                          const char* source);
    wgpu::ShaderModule CreateShaderModuleFromASM(const wgpu::Device& device, const char* source);

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
                                              uint32_t rowPitch,
                                              uint32_t imageHeight);
    wgpu::TextureCopyView CreateTextureCopyView(wgpu::Texture texture,
                                                uint32_t level,
                                                uint32_t slice,
                                                wgpu::Origin3D origin);

    struct ComboRenderPassDescriptor : public wgpu::RenderPassDescriptor {
      public:
        ComboRenderPassDescriptor(std::initializer_list<wgpu::TextureView> colorAttachmentInfo,
                                  wgpu::TextureView depthStencil = wgpu::TextureView());
        const ComboRenderPassDescriptor& operator=(
            const ComboRenderPassDescriptor& otherRenderPass);

        std::array<wgpu::RenderPassColorAttachmentDescriptor, kMaxColorAttachments>
            cColorAttachments;
        wgpu::RenderPassDepthStencilAttachmentDescriptor cDepthStencilAttachmentInfo;
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

    wgpu::SamplerDescriptor GetDefaultSamplerDescriptor();
    wgpu::PipelineLayout MakeBasicPipelineLayout(const wgpu::Device& device,
                                                 const wgpu::BindGroupLayout* bindGroupLayout);
    wgpu::BindGroupLayout MakeBindGroupLayout(
        const wgpu::Device& device,
        std::initializer_list<wgpu::BindGroupLayoutBinding> bindingsInitializer);

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

        wgpu::BindGroupBinding GetAsBinding() const;

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
        std::initializer_list<BindingInitializationHelper> bindingsInitializer);

}  // namespace utils

#endif  // UTILS_DAWNHELPERS_H_
