// Copyright 2018 The Dawn Authors
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

#include <cstdint>
#include <string>
#include <vector>

#include <spirv_hlsl.hpp>

#include "DawnSPIRVCrossFuzzer.h"

namespace {

    int FuzzTask(const std::vector<uint32_t>& input) {
        // Values come from ShaderModuleD3D12.cpp
        spirv_cross::CompilerGLSL::Options options_glsl;
        // Force all uninitialized variables to be 0, otherwise they will fail to compile
        // by FXC.
        options_glsl.force_zero_initialized_variables = true;

        spirv_cross::CompilerHLSL::Options options_hlsl;
        options_hlsl.shader_model = 51;
        options_hlsl.point_coord_compat = true;
        options_hlsl.point_size_compat = true;
        options_hlsl.nonwritable_uav_texture_as_srv = true;

        spirv_cross::CompilerHLSL compiler(input);
        compiler.set_common_options(options_glsl);
        compiler.set_hlsl_options(options_hlsl);
        compiler.compile();

        return 0;
    }
}  // namespace

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    return DawnSPIRVCrossFuzzer::Run(data, size, FuzzTask);
}
