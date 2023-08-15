// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_RAISE_SHADER_IO_H_
#define SRC_TINT_LANG_SPIRV_WRITER_RAISE_SHADER_IO_H_

#include <string>

#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::spirv::writer::raise {

/// ShaderIOConfig describes the set of configuration options for the ShaderIO transform.
struct ShaderIOConfig {
    /// true if frag_depth builtin outputs should be clamped
    bool clamp_frag_depth = false;
};

/// ShaderIO is a transform that modifies each entry point function's parameters and return
/// value to prepare them for SPIR-V codegen.
/// @param module the module to transform
/// @param config the configuration
/// @returns an error string on failure
Result<SuccessType, std::string> ShaderIO(core::ir::Module* module, const ShaderIOConfig& config);

}  // namespace tint::spirv::writer::raise

#endif  // SRC_TINT_LANG_SPIRV_WRITER_RAISE_SHADER_IO_H_
