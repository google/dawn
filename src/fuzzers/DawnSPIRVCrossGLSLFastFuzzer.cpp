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

#include "DawnSPIRVCrossFuzzer.h"

namespace {

    int GLSLFastFuzzTask(std::vector<uint32_t> input) {
        spirv_cross::CompilerGLSL compiler(input);

        // Using the options that are used by Dawn, they appear in ShaderModuleGL.cpp
        spirv_cross::CompilerGLSL::Options options;
        options.version = 440;
        compiler.set_common_options(options);

        std::string result = compiler.compile();
        return 0;
    }

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    return DawnSPIRVCrossFuzzer::Run(data, size, GLSLFastFuzzTask);
}
