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

#include "utils/DawnHelpers.h"

#include "common/Assert.h"

#include <shaderc/shaderc.hpp>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace utils {

    namespace {

        shaderc_shader_kind ShadercShaderKind(dawn::ShaderStage stage) {
            switch (stage) {
                case dawn::ShaderStage::Vertex:
                    return shaderc_glsl_vertex_shader;
                case dawn::ShaderStage::Fragment:
                    return shaderc_glsl_fragment_shader;
                case dawn::ShaderStage::Compute:
                    return shaderc_glsl_compute_shader;
                default:
                    UNREACHABLE();
            }
        }

        dawn::ShaderModule CreateShaderModuleFromResult(
            const dawn::Device& device,
            const shaderc::SpvCompilationResult& result) {
            // result.cend and result.cbegin return pointers to uint32_t.
            const uint32_t* resultBegin = result.cbegin();
            const uint32_t* resultEnd = result.cend();
            // So this size is in units of sizeof(uint32_t).
            ptrdiff_t resultSize = resultEnd - resultBegin;
            // SetSource takes data as uint32_t*.

            dawn::ShaderModuleDescriptor descriptor;
            descriptor.codeSize = static_cast<uint32_t>(resultSize);
            descriptor.code = result.cbegin();
            return device.CreateShaderModule(&descriptor);
        }

    }  // anonymous namespace

    dawn::ShaderModule CreateShaderModule(const dawn::Device& device,
                                          dawn::ShaderStage stage,
                                          const char* source) {
        shaderc_shader_kind kind = ShadercShaderKind(stage);

        shaderc::Compiler compiler;
        auto result = compiler.CompileGlslToSpv(source, strlen(source), kind, "myshader?");
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << result.GetErrorMessage();
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

    dawn::ShaderModule CreateShaderModuleFromASM(const dawn::Device& device, const char* source) {
        shaderc::Compiler compiler;
        shaderc::SpvCompilationResult result = compiler.AssembleToSpv(source, strlen(source));
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << result.GetErrorMessage();
            return {};
        }

        return CreateShaderModuleFromResult(device, result);
    }

    dawn::Buffer CreateBufferFromData(const dawn::Device& device,
                                      const void* data,
                                      uint32_t size,
                                      dawn::BufferUsageBit usage) {
        dawn::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage | dawn::BufferUsageBit::TransferDst;

        dawn::Buffer buffer = device.CreateBuffer(&descriptor);
        buffer.SetSubData(0, size, reinterpret_cast<const uint8_t*>(data));
        return buffer;
    }

    BasicRenderPass CreateBasicRenderPass(const dawn::Device& device,
                                          uint32_t width,
                                          uint32_t height) {
        BasicRenderPass result;
        result.width = width;
        result.height = height;

        result.colorFormat = dawn::TextureFormat::R8G8B8A8Unorm;
        dawn::TextureDescriptor descriptor;
        descriptor.dimension = dawn::TextureDimension::e2D;
        descriptor.size.width = width;
        descriptor.size.height = height;
        descriptor.size.depth = 1;
        descriptor.arrayLayer = 1;
        descriptor.format = result.colorFormat;
        descriptor.levelCount = 1;
        descriptor.usage =
            dawn::TextureUsageBit::OutputAttachment | dawn::TextureUsageBit::TransferSrc;
        result.color = device.CreateTexture(&descriptor);

        dawn::TextureView colorView = result.color.CreateDefaultTextureView();
        result.renderPassInfo = device.CreateRenderPassDescriptorBuilder()
                                    .SetColorAttachment(0, colorView, dawn::LoadOp::Clear)
                                    .GetResult();

        return result;
    }

    dawn::BufferCopyView CreateBufferCopyView(dawn::Buffer buffer,
                                              uint32_t offset,
                                              uint32_t rowPitch,
                                              uint32_t imageHeight) {
        dawn::BufferCopyView bufferCopyView;
        bufferCopyView.buffer = buffer;
        bufferCopyView.offset = offset;
        bufferCopyView.rowPitch = rowPitch;
        bufferCopyView.imageHeight = imageHeight;

        return bufferCopyView;
    }

    dawn::TextureCopyView CreateTextureCopyView(dawn::Texture texture,
                                                uint32_t level,
                                                uint32_t slice,
                                                dawn::Origin3D origin,
                                                dawn::TextureAspect aspect) {
        dawn::TextureCopyView textureCopyView;
        textureCopyView.texture = texture;
        textureCopyView.level = level;
        textureCopyView.slice = slice;
        textureCopyView.origin = origin;
        textureCopyView.aspect = aspect;

        return textureCopyView;
    }

    dawn::SamplerDescriptor GetDefaultSamplerDescriptor() {
        dawn::SamplerDescriptor desc;

        desc.minFilter = dawn::FilterMode::Linear;
        desc.magFilter = dawn::FilterMode::Linear;
        desc.mipmapFilter = dawn::FilterMode::Linear;
        desc.addressModeU = dawn::AddressMode::Repeat;
        desc.addressModeV = dawn::AddressMode::Repeat;
        desc.addressModeW = dawn::AddressMode::Repeat;

        return desc;
    }

    dawn::PipelineLayout MakeBasicPipelineLayout(const dawn::Device& device,
                                                 const dawn::BindGroupLayout* bindGroupLayout) {
        dawn::PipelineLayoutDescriptor descriptor;
        if (bindGroupLayout) {
            descriptor.numBindGroupLayouts = 1;
            descriptor.bindGroupLayouts = bindGroupLayout;
        } else {
            descriptor.numBindGroupLayouts = 0;
            descriptor.bindGroupLayouts = nullptr;
        }
        return device.CreatePipelineLayout(&descriptor);
    }

    dawn::BindGroupLayout MakeBindGroupLayout(
        const dawn::Device& device,
        std::initializer_list<dawn::BindGroupLayoutBinding> bindingsInitializer) {
        constexpr dawn::ShaderStageBit kNoStages{};

        std::vector<dawn::BindGroupLayoutBinding> bindings;
        for (const dawn::BindGroupLayoutBinding& binding : bindingsInitializer) {
            if (binding.visibility != kNoStages) {
                bindings.push_back(binding);
            }
        }

        dawn::BindGroupLayoutDescriptor descriptor;
        descriptor.numBindings = static_cast<uint32_t>(bindings.size());
        descriptor.bindings = bindings.data();
        return device.CreateBindGroupLayout(&descriptor);
    }

}  // namespace utils
