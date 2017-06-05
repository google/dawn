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
        options.shader_model = 40;
        options.flip_vert_y = false;
        options.fixup_clipspace = true;

        compiler.set_options(options);

        ExtractSpirvInfo(compiler);

        hlslSource = compiler.compile();
    }

    const std::string& ShaderModule::GetHLSLSource() const {
        return hlslSource;
    }

}
}
