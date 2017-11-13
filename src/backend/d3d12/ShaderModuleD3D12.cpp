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

#include "backend/d3d12/ShaderModuleD3D12.h"

#include <spirv-cross/spirv_hlsl.hpp>

namespace backend {
namespace d3d12 {

    ShaderModule::ShaderModule(Device* device, ShaderModuleBuilder* builder)
        : ShaderModuleBase(builder), device(device) {
        spirv_cross::CompilerHLSL compiler(builder->AcquireSpirv());

        spirv_cross::CompilerGLSL::Options options_glsl;
        options_glsl.vertex.flip_vert_y = false;
        options_glsl.vertex.fixup_clipspace = true;
        compiler.spirv_cross::CompilerGLSL::set_options(options_glsl);

        spirv_cross::CompilerHLSL::Options options_hlsl;
        options_hlsl.shader_model = 51;
        compiler.spirv_cross::CompilerHLSL::set_options(options_hlsl);

        ExtractSpirvInfo(compiler);

        // rename bindings so that each register type b/u/t/s starts at 0 and then offset by kMaxBindingsPerGroup * bindGroupIndex
        auto RenumberBindings = [&](std::vector<spirv_cross::Resource> resources) {
            std::array<uint32_t, kMaxBindGroups> baseRegisters = {};

            for (const auto& resource : resources) {
                auto bindGroupIndex = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                auto& baseRegister = baseRegisters[bindGroupIndex];
                auto bindGroupOffset = bindGroupIndex * kMaxBindingsPerGroup;
                compiler.set_decoration(resource.id, spv::DecorationBinding, bindGroupOffset + baseRegister++);
            }
        };

        const auto& resources = compiler.get_shader_resources();
        RenumberBindings(resources.uniform_buffers);    // c
        RenumberBindings(resources.storage_buffers);    // u
        RenumberBindings(resources.separate_images);    // t
        RenumberBindings(resources.separate_samplers);  // s

        hlslSource = compiler.compile();
    }

    const std::string& ShaderModule::GetHLSLSource() const {
        return hlslSource;
    }

}
}
