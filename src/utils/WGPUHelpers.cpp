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

#include "utils/WGPUHelpers.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Log.h"

#include <shaderc/shaderc.hpp>

#include <cstring>
#include <iomanip>
#include <sstream>

namespace utils {

    namespace {

        shaderc_shader_kind ShadercShaderKind(SingleShaderStage stage) {
            switch (stage) {
                case SingleShaderStage::Vertex:
                    return shaderc_glsl_vertex_shader;
                case SingleShaderStage::Fragment:
                    return shaderc_glsl_fragment_shader;
                case SingleShaderStage::Compute:
                    return shaderc_glsl_compute_shader;
                default:
                    UNREACHABLE();
            }
        }

        wgpu::ShaderModule CreateShaderModuleFromResult(
            const wgpu::Device& device,
            const shaderc::SpvCompilationResult& result) {
            // result.cend and result.cbegin return pointers to uint32_t.
            const uint32_t* resultBegin = result.cbegin();
            const uint32_t* resultEnd = result.cend();
            // So this size is in units of sizeof(uint32_t).
            ptrdiff_t resultSize = resultEnd - resultBegin;
            // SetSource takes data as uint32_t*.

            wgpu::ShaderModuleDescriptor descriptor;
            descriptor.codeSize = static_cast<uint32_t>(resultSize);
            descriptor.code = result.cbegin();
            return device.CreateShaderModule(&descriptor);
        }

    }  // anonymous namespace

    wgpu::ShaderModule CreateShaderModule(const wgpu::Device& device,
                                          SingleShaderStage stage,
                                          const char* source) {
        shaderc_shader_kind kind = ShadercShaderKind(stage);

        shaderc::Compiler compiler;
        auto result = compiler.CompileGlslToSpv(source, strlen(source), kind, "myshader?");
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            dawn::ErrorLog() << result.GetErrorMessage();
            return {};
        }
#ifdef DUMP_SPIRV_ASSEMBLY
        {
            shaderc::CompileOptions options;
            auto resultAsm = compiler.CompileGlslToSpvAssembly(source, strlen(source), kind,
                                                               "myshader?", options);
            size_t sizeAsm = (resultAsm.cend() - resultAsm.cbegin());

            char* buffer = reinterpret_cast<char*>(malloc(sizeAsm + 1));
            memcpy(buffer, resultAsm.cbegin(), sizeAsm);
            buffer[sizeAsm] = '\0';
            printf("SPIRV ASSEMBLY DUMP START\n%s\nSPIRV ASSEMBLY DUMP END\n", buffer);
            free(buffer);
        }
#endif

#ifdef DUMP_SPIRV_JS_ARRAY
        printf("SPIRV JS ARRAY DUMP START\n");
        for (size_t i = 0; i < size; i++) {
            printf("%#010x", result.cbegin()[i]);
            if ((i + 1) % 4 == 0) {
                printf(",\n");
            } else {
                printf(", ");
            }
        }
        printf("\n");
        printf("SPIRV JS ARRAY DUMP END\n");
#endif

        return CreateShaderModuleFromResult(device, result);
    }

    wgpu::ShaderModule CreateShaderModuleFromASM(const wgpu::Device& device, const char* source) {
        shaderc::Compiler compiler;
        shaderc::SpvCompilationResult result = compiler.AssembleToSpv(source, strlen(source));
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            dawn::ErrorLog() << result.GetErrorMessage();
            return {};
        }

        return CreateShaderModuleFromResult(device, result);
    }

    wgpu::Buffer CreateBufferFromData(const wgpu::Device& device,
                                      const void* data,
                                      uint64_t size,
                                      wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage | wgpu::BufferUsage::CopyDst;

        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        buffer.SetSubData(0, size, data);
        return buffer;
    }

    ComboRenderPassDescriptor::ComboRenderPassDescriptor(
        std::initializer_list<wgpu::TextureView> colorAttachmentInfo,
        wgpu::TextureView depthStencil) {
        for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
            cColorAttachments[i].loadOp = wgpu::LoadOp::Clear;
            cColorAttachments[i].storeOp = wgpu::StoreOp::Store;
            cColorAttachments[i].clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
        }

        cDepthStencilAttachmentInfo.clearDepth = 1.0f;
        cDepthStencilAttachmentInfo.clearStencil = 0;
        cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
        cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
        cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;

        colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfo.size());
        uint32_t colorAttachmentIndex = 0;
        for (const wgpu::TextureView& colorAttachment : colorAttachmentInfo) {
            if (colorAttachment.Get() != nullptr) {
                cColorAttachments[colorAttachmentIndex].attachment = colorAttachment;
            }
            ++colorAttachmentIndex;
        }
        colorAttachments = cColorAttachments.data();

        if (depthStencil.Get() != nullptr) {
            cDepthStencilAttachmentInfo.attachment = depthStencil;
            depthStencilAttachment = &cDepthStencilAttachmentInfo;
        } else {
            depthStencilAttachment = nullptr;
        }
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

    BasicRenderPass::BasicRenderPass()
        : width(0),
          height(0),
          color(nullptr),
          colorFormat(wgpu::TextureFormat::RGBA8Unorm),
          renderPassInfo({}) {
    }

    BasicRenderPass::BasicRenderPass(uint32_t texWidth,
                                     uint32_t texHeight,
                                     wgpu::Texture colorAttachment,
                                     wgpu::TextureFormat textureFormat)
        : width(texWidth),
          height(texHeight),
          color(colorAttachment),
          colorFormat(textureFormat),
          renderPassInfo({colorAttachment.CreateView()}) {
    }

    BasicRenderPass CreateBasicRenderPass(const wgpu::Device& device,
                                          uint32_t width,
                                          uint32_t height) {
        DAWN_ASSERT(width > 0 && height > 0);

        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = width;
        descriptor.size.height = height;
        descriptor.size.depth = 1;
        descriptor.arrayLayerCount = 1;
        descriptor.sampleCount = 1;
        descriptor.format = BasicRenderPass::kDefaultColorFormat;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture color = device.CreateTexture(&descriptor);

        return BasicRenderPass(width, height, color);
    }

    wgpu::BufferCopyView CreateBufferCopyView(wgpu::Buffer buffer,
                                              uint64_t offset,
                                              uint32_t rowPitch,
                                              uint32_t imageHeight) {
        wgpu::BufferCopyView bufferCopyView;
        bufferCopyView.buffer = buffer;
        bufferCopyView.offset = offset;
        bufferCopyView.rowPitch = rowPitch;
        bufferCopyView.imageHeight = imageHeight;

        return bufferCopyView;
    }

    wgpu::TextureCopyView CreateTextureCopyView(wgpu::Texture texture,
                                                uint32_t mipLevel,
                                                uint32_t arrayLayer,
                                                wgpu::Origin3D origin) {
        wgpu::TextureCopyView textureCopyView;
        textureCopyView.texture = texture;
        textureCopyView.mipLevel = mipLevel;
        textureCopyView.arrayLayer = arrayLayer;
        textureCopyView.origin = origin;

        return textureCopyView;
    }

    wgpu::SamplerDescriptor GetDefaultSamplerDescriptor() {
        wgpu::SamplerDescriptor desc;

        desc.minFilter = wgpu::FilterMode::Linear;
        desc.magFilter = wgpu::FilterMode::Linear;
        desc.mipmapFilter = wgpu::FilterMode::Linear;
        desc.addressModeU = wgpu::AddressMode::Repeat;
        desc.addressModeV = wgpu::AddressMode::Repeat;
        desc.addressModeW = wgpu::AddressMode::Repeat;
        desc.lodMinClamp = kLodMin;
        desc.lodMaxClamp = kLodMax;
        desc.compare = wgpu::CompareFunction::Never;

        return desc;
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

    wgpu::BindGroupLayout MakeBindGroupLayout(
        const wgpu::Device& device,
        std::initializer_list<wgpu::BindGroupLayoutBinding> bindingsInitializer) {
        constexpr wgpu::ShaderStage kNoStages{};

        std::vector<wgpu::BindGroupLayoutBinding> bindings;
        for (const wgpu::BindGroupLayoutBinding& binding : bindingsInitializer) {
            if (binding.visibility != kNoStages) {
                bindings.push_back(binding);
            }
        }

        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptor.bindings = bindings.data();
        return device.CreateBindGroupLayout(&descriptor);
    }

    BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                             const wgpu::Sampler& sampler)
        : binding(binding), sampler(sampler) {
    }

    BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                             const wgpu::TextureView& textureView)
        : binding(binding), textureView(textureView) {
    }

    BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                             const wgpu::Buffer& buffer,
                                                             uint64_t offset,
                                                             uint64_t size)
        : binding(binding), buffer(buffer), offset(offset), size(size) {
    }

    wgpu::BindGroupBinding BindingInitializationHelper::GetAsBinding() const {
        wgpu::BindGroupBinding result;

        result.binding = binding;
        result.sampler = sampler;
        result.textureView = textureView;
        result.buffer = buffer;
        result.offset = offset;
        result.size = size;

        return result;
    }

    wgpu::BindGroup MakeBindGroup(
        const wgpu::Device& device,
        const wgpu::BindGroupLayout& layout,
        std::initializer_list<BindingInitializationHelper> bindingsInitializer) {
        std::vector<wgpu::BindGroupBinding> bindings;
        for (const BindingInitializationHelper& helper : bindingsInitializer) {
            bindings.push_back(helper.GetAsBinding());
        }

        wgpu::BindGroupDescriptor descriptor;
        descriptor.layout = layout;
        descriptor.bindingCount = bindings.size();
        descriptor.bindings = bindings.data();

        return device.CreateBindGroup(&descriptor);
    }

}  // namespace utils
