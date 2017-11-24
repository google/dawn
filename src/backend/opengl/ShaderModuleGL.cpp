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

#include "backend/opengl/ShaderModuleGL.h"

#include "common/Assert.h"
#include "common/Platform.h"

#include <spirv-cross/spirv_glsl.hpp>

#include <sstream>

namespace backend { namespace opengl {

    std::string GetBindingName(uint32_t group, uint32_t binding) {
        std::ostringstream o;
        o << "nxt_binding_" << group << "_" << binding;
        return o.str();
    }

    bool operator<(const BindingLocation& a, const BindingLocation& b) {
        return std::tie(a.group, a.binding) < std::tie(b.group, b.binding);
    }

    bool operator<(const CombinedSampler& a, const CombinedSampler& b) {
        return std::tie(a.samplerLocation, a.textureLocation) <
               std::tie(b.samplerLocation, b.textureLocation);
    }

    std::string CombinedSampler::GetName() const {
        std::ostringstream o;
        o << "nxt_combined";
        o << "_" << samplerLocation.group << "_" << samplerLocation.binding;
        o << "_with_" << textureLocation.group << "_" << textureLocation.binding;
        return o.str();
    }

    ShaderModule::ShaderModule(ShaderModuleBuilder* builder) : ShaderModuleBase(builder) {
        spirv_cross::CompilerGLSL compiler(builder->AcquireSpirv());
        spirv_cross::CompilerGLSL::Options options;

        // TODO(cwallez@chromium.org): discover the backing context version and use that.
#if defined(NXT_PLATFORM_APPLE)
        options.version = 410;
#else
        options.version = 440;
#endif
        options.vertex.flip_vert_y = true;
        compiler.set_options(options);

        // Rename the push constant block to be prefixed with the shader stage type so that uniform
        // names don't match between the FS and the VS.
        const auto& resources = compiler.get_shader_resources();
        if (resources.push_constant_buffers.size() > 0) {
            const char* prefix = nullptr;
            switch (compiler.get_execution_model()) {
                case spv::ExecutionModelVertex:
                    prefix = "vs_";
                    break;
                case spv::ExecutionModelFragment:
                    prefix = "fs_";
                    break;
                case spv::ExecutionModelGLCompute:
                    prefix = "cs_";
                    break;
                default:
                    UNREACHABLE();
            }
            auto interfaceBlock = resources.push_constant_buffers[0];
            compiler.set_name(interfaceBlock.id, prefix + interfaceBlock.name);
        }

        ExtractSpirvInfo(compiler);

        const auto& bindingInfo = GetBindingInfo();

        // Extract bindings names so that it can be used to get its location in program.
        // Now translate the separate sampler / textures into combined ones and store their info.
        // We need to do this before removing the set and binding decorations.
        compiler.build_combined_image_samplers();

        for (const auto& combined : compiler.get_combined_image_samplers()) {
            mCombinedInfo.emplace_back();

            auto& info = mCombinedInfo.back();
            info.samplerLocation.group =
                compiler.get_decoration(combined.sampler_id, spv::DecorationDescriptorSet);
            info.samplerLocation.binding =
                compiler.get_decoration(combined.sampler_id, spv::DecorationBinding);
            info.textureLocation.group =
                compiler.get_decoration(combined.image_id, spv::DecorationDescriptorSet);
            info.textureLocation.binding =
                compiler.get_decoration(combined.image_id, spv::DecorationBinding);
            compiler.set_name(combined.combined_id, info.GetName());
        }

        // Change binding names to be "nxt_binding_<group>_<binding>".
        // Also unsets the SPIRV "Binding" decoration as it outputs "layout(binding=)" which
        // isn't supported on OSX's OpenGL.
        for (uint32_t group = 0; group < kMaxBindGroups; ++group) {
            for (uint32_t binding = 0; binding < kMaxBindingsPerGroup; ++binding) {
                const auto& info = bindingInfo[group][binding];
                if (info.used) {
                    compiler.set_name(info.base_type_id, GetBindingName(group, binding));
                    compiler.unset_decoration(info.id, spv::DecorationBinding);
                    compiler.unset_decoration(info.id, spv::DecorationDescriptorSet);
                }
            }
        }

        mGlslSource = compiler.compile();
    }

    const char* ShaderModule::GetSource() const {
        return reinterpret_cast<const char*>(mGlslSource.data());
    }

    const ShaderModule::CombinedSamplerInfo& ShaderModule::GetCombinedSamplerInfo() const {
        return mCombinedInfo;
    }

}}  // namespace backend::opengl
