// Copyright 2017 The NXT Authors
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

#include "utils/NXTHelpers.h"

#include "common/Assert.h"

#include <shaderc/shaderc.hpp>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace utils {

    void FillShaderModuleBuilder(const nxt::ShaderModuleBuilder& builder, nxt::ShaderStage stage, const char* source) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        shaderc_shader_kind kind;
        switch (stage) {
            case nxt::ShaderStage::Vertex:
                kind = shaderc_glsl_vertex_shader;
                break;
            case nxt::ShaderStage::Fragment:
                kind = shaderc_glsl_fragment_shader;
                break;
            case nxt::ShaderStage::Compute:
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
            auto resultAsm = compiler.CompileGlslToSpvAssembly(source, strlen(source), kind, "myshader?", options);
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

    nxt::ShaderModule CreateShaderModule(const nxt::Device& device, nxt::ShaderStage stage, const char* source) {
        nxt::ShaderModuleBuilder builder = device.CreateShaderModuleBuilder();
        FillShaderModuleBuilder(builder, stage, source);
        return builder.GetResult();
    }

    void CreateDefaultRenderPass(const nxt::Device& device, nxt::RenderPass* renderPass, nxt::Framebuffer* framebuffer) {
        auto depthStencilTexture = device.CreateTextureBuilder()
            .SetDimension(nxt::TextureDimension::e2D)
            .SetExtent(640, 480, 1)
            .SetFormat(nxt::TextureFormat::D32FloatS8Uint)
            .SetMipLevels(1)
            .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment)
            .GetResult();
        depthStencilTexture.FreezeUsage(nxt::TextureUsageBit::OutputAttachment);
        auto depthStencilView = depthStencilTexture.CreateTextureViewBuilder()
            .GetResult();

        *renderPass = device.CreateRenderPassBuilder()
            .SetAttachmentCount(2)
            .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
            .AttachmentSetFormat(1, nxt::TextureFormat::D32FloatS8Uint)
            .SetSubpassCount(1)
            .SubpassSetColorAttachment(0, 0, 0)
            .SubpassSetDepthStencilAttachment(0, 1)
            .GetResult();
        *framebuffer = device.CreateFramebufferBuilder()
            .SetRenderPass(*renderPass)
            .SetDimensions(640, 480)
            // Attachment 0 is implicit until we add WSI
            .SetAttachment(1, depthStencilView)
            .GetResult();
    }

    nxt::Buffer CreateFrozenBufferFromData(const nxt::Device& device, const void* data, uint32_t size, nxt::BufferUsageBit usage) {
        nxt::Buffer buffer = device.CreateBufferBuilder()
            .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | usage)
            .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
            .SetSize(size)
            .GetResult();
        buffer.SetSubData(0, size / sizeof(uint32_t), reinterpret_cast<const uint32_t*>(data));
        buffer.FreezeUsage(usage);
        return buffer;
    }

}
