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

#include "dawn_native/d3d12/ShaderModuleD3D12.h"

#include "common/Assert.h"
#include "dawn_native/d3d12/DeviceD3D12.h"

#include <spirv-cross/spirv_hlsl.hpp>

namespace dawn_native { namespace d3d12 {

    // TODO(kainino@chromium.org): Consider replacing this with a generic enum_map.
    template <typename T>
    class BindingTypeMap {
      public:
        T& operator[](dawn::BindingType type) {
            switch (type) {
                case dawn::BindingType::UniformBuffer:
                    return mMap[0];
                case dawn::BindingType::Sampler:
                    return mMap[1];
                case dawn::BindingType::SampledTexture:
                    return mMap[2];
                case dawn::BindingType::StorageBuffer:
                    return mMap[3];
                default:
                    DAWN_UNREACHABLE();
            }
        }

      private:
        static constexpr int kNumBindingTypes = 4;
        std::array<T, kNumBindingTypes> mMap{};
    };

    ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
        : ShaderModuleBase(device, descriptor) {
        spirv_cross::CompilerHLSL compiler(descriptor->code, descriptor->codeSize);

        spirv_cross::CompilerGLSL::Options options_glsl;
        options_glsl.vertex.fixup_clipspace = true;
        options_glsl.vertex.flip_vert_y = true;
        compiler.set_common_options(options_glsl);

        spirv_cross::CompilerHLSL::Options options_hlsl;
        options_hlsl.shader_model = 51;
        compiler.set_hlsl_options(options_hlsl);

        ExtractSpirvInfo(compiler);

        // rename bindings so that each register type c/u/t/s starts at 0 and then offset by
        // kMaxBindingsPerGroup * bindGroupIndex
        const auto& moduleBindingInfo = GetBindingInfo();
        for (uint32_t group = 0; group < moduleBindingInfo.size(); ++group) {
            const auto& groupBindingInfo = moduleBindingInfo[group];

            BindingTypeMap<uint32_t> baseRegisters{};
            for (const auto& bindingInfo : groupBindingInfo) {
                if (bindingInfo.used) {
                    uint32_t& baseRegister = baseRegisters[bindingInfo.type];
                    uint32_t bindGroupOffset = group * kMaxBindingsPerGroup;
                    compiler.set_decoration(bindingInfo.id, spv::DecorationBinding,
                                            bindGroupOffset + baseRegister++);
                }
            }
        }

        mHlslSource = compiler.compile();
    }

    const std::string& ShaderModule::GetHLSLSource() const {
        return mHlslSource;
    }

}}  // namespace dawn_native::d3d12
