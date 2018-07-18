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

    void FillShaderModuleBuilder(const dawn::ShaderModuleBuilder& builder,
                                 dawn::ShaderStage stage,
                                 const char* source) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        shaderc_shader_kind kind;
        switch (stage) {
            case dawn::ShaderStage::Vertex:
                kind = shaderc_glsl_vertex_shader;
                break;
            case dawn::ShaderStage::Fragment:
                kind = shaderc_glsl_fragment_shader;
                break;
            case dawn::ShaderStage::Compute:
                kind = shaderc_glsl_compute_shader;
                break;
            default:
                UNREACHABLE();
        }

        auto result = compiler.CompileGlslToSpv(source, strlen(source), kind, "myshader?", options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << result.GetErrorMessage();
            return;
        }

        // result.cend and result.cbegin return pointers to uint32_t.
        const uint32_t* resultBegin = result.cbegin();
        const uint32_t* resultEnd = result.cend();
        // So this size is in units of sizeof(uint32_t).
        ptrdiff_t resultSize = resultEnd - resultBegin;
        // SetSource takes data as uint32_t*.
        builder.SetSource(static_cast<uint32_t>(resultSize), result.cbegin());

#ifdef DUMP_SPIRV_ASSEMBLY
        {
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
    }

    dawn::ShaderModule CreateShaderModule(const dawn::Device& device,
                                          dawn::ShaderStage stage,
                                          const char* source) {
        dawn::ShaderModuleBuilder builder = device.CreateShaderModuleBuilder();
        FillShaderModuleBuilder(builder, stage, source);
        return builder.GetResult();
    }

    dawn::Buffer CreateBufferFromData(const dawn::Device& device,
                                      const void* data,
                                      uint32_t size,
                                      dawn::BufferUsageBit usage) {
        dawn::Buffer buffer = device.CreateBufferBuilder()
                                  .SetAllowedUsage(dawn::BufferUsageBit::TransferDst | usage)
                                  .SetSize(size)
                                  .GetResult();
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
        result.color = device.CreateTextureBuilder()
                           .SetDimension(dawn::TextureDimension::e2D)
                           .SetExtent(width, height, 1)
                           .SetFormat(result.colorFormat)
                           .SetMipLevels(1)
                           .SetAllowedUsage(dawn::TextureUsageBit::OutputAttachment |
                                            dawn::TextureUsageBit::TransferSrc)
                           .GetResult();

        dawn::TextureView colorView = result.color.CreateTextureViewBuilder().GetResult();
        result.renderPassInfo = device.CreateRenderPassDescriptorBuilder()
                                    .SetColorAttachment(0, colorView, dawn::LoadOp::Clear)
                                    .GetResult();

        return result;
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
        std::initializer_list<dawn::BindGroupBinding> bindingsInitializer) {
        std::vector<dawn::BindGroupBinding> bindings;
        dawn::ShaderStageBit kNoStages{};
        for (const dawn::BindGroupBinding& binding : bindingsInitializer) {
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
