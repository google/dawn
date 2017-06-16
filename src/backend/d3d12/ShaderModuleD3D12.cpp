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

#include "ShaderModuleD3D12.h"

#include <spirv-cross/spirv_hlsl.hpp>

namespace backend {
namespace d3d12 {

    ShaderModule::ShaderModule(Device* device, ShaderModuleBuilder* builder)
        : ShaderModuleBase(builder), device(device) {
        spirv_cross::CompilerHLSL compiler(builder->AcquireSpirv());

        spirv_cross::CompilerHLSL::Options options;
        options.shader_model = 51;
        options.flip_vert_y = false;
        options.fixup_clipspace = true;

        compiler.set_options(options);

        ExtractSpirvInfo(compiler);

        enum RegisterType {
            Buffer,
            UnorderedAccess,
            Texture,
            Sampler,
            Count,
        };

        std::array<uint32_t, RegisterType::Count * kMaxBindGroups> baseRegisters = {};

        const auto& resources = compiler.get_shader_resources();

        // rename bindings so that each register type b/u/t/s starts at 0 and then offset by kMaxBindingsPerGroup * bindGroupIndex
        auto RenumberBindings = [&](std::vector<spirv_cross::Resource> resources, uint32_t offset) {
            for (const auto& resource : resources) {
                auto bindGroupIndex = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                auto& baseRegister = baseRegisters[RegisterType::Count * bindGroupIndex + offset];
                auto bindGroupOffset = bindGroupIndex * kMaxBindingsPerGroup;
                compiler.set_decoration(resource.id, spv::DecorationBinding, bindGroupOffset + baseRegister++);
            }
        };

        RenumberBindings(resources.uniform_buffers, RegisterType::Buffer);
        RenumberBindings(resources.storage_buffers, RegisterType::UnorderedAccess);
        RenumberBindings(resources.separate_images, RegisterType::Texture);
        RenumberBindings(resources.separate_samplers, RegisterType::Sampler);

        hlslSource = compiler.compile();

        {
            // pending https://github.com/KhronosGroup/SPIRV-Cross/issues/216
            // rename ": register(cN)" to ": register(bN)"
            std::string::size_type pos = 0;
            const std::string search = ": register(c";
            const std::string replace = ": register(b";
            while ((pos = hlslSource.find(search, pos)) != std::string::npos) {
                hlslSource.replace(pos, search.length(), replace);
                pos += replace.length();
            }
        }
    }

    const std::string& ShaderModule::GetHLSLSource() const {
        return hlslSource;
    }

}
}
