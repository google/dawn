// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_LANG_MSL_VALIDATE_VAL_H_
#define SRC_TINT_LANG_MSL_VALIDATE_VAL_H_

#include <string>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/pipeline_stage.h"

// Forward declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::msl::validate {

using EntryPointList = std::vector<std::pair<std::string, ast::PipelineStage>>;

// The version of MSL to validate against.
enum class MslVersion {
    kMsl_1_2,
    kMsl_2_1,
};

/// The return structure of Validate()
struct Result {
    /// True if validation passed
    bool failed = false;
    /// Output of Metal compiler.
    std::string output;
};

/// Msl attempts to compile the shader with the Metal Shader Compiler,
/// verifying that the shader compiles successfully.
/// @param xcrun_path path to xcrun
/// @param source the generated MSL source
/// @param version the version of MSL to validate against
/// @return the result of the compile
Result Msl(const std::string& xcrun_path, const std::string& source, MslVersion version);

#ifdef TINT_ENABLE_MSL_VALIDATION_USING_METAL_API
/// Msl attempts to compile the shader with the runtime Metal Shader Compiler
/// API, verifying that the shader compiles successfully.
/// @param source the generated MSL source
/// @param version the version of MSL to validate against
/// @return the result of the compile
Result UsingMetalAPI(const std::string& source, MslVersion version);
#endif  // TINT_ENABLE_MSL_VALIDATION_USING_METAL_API

}  // namespace tint::msl::validate

#endif  // SRC_TINT_LANG_MSL_VALIDATE_VAL_H_
