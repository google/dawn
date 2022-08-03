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

#ifndef SRC_TINT_VAL_VAL_H_
#define SRC_TINT_VAL_VAL_H_

#include <string>
#include <utility>
#include <vector>

#include "src/tint/ast/pipeline_stage.h"

// Forward declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::val {

using EntryPointList = std::vector<std::pair<std::string, ast::PipelineStage>>;

/// Name of the FXC compiler DLL
static constexpr const char kFxcDLLName[] = "d3dcompiler_47.dll";

/// The return structure of Validate()
struct Result {
    /// True if validation passed
    bool failed = false;
    /// Output of DXC.
    std::string output;
};

/// Hlsl attempts to compile the shader with DXC, verifying that the shader
/// compiles successfully.
/// @param dxc_path path to DXC
/// @param source the generated HLSL source
/// @param entry_points the list of entry points to validate
/// @return the result of the compile
Result HlslUsingDXC(const std::string& dxc_path,
                    const std::string& source,
                    const EntryPointList& entry_points,
                    bool require_16bit_types);

#ifdef _WIN32
/// Hlsl attempts to compile the shader with FXC, verifying that the shader
/// compiles successfully.
/// @param fxc_path path to the FXC DLL
/// @param source the generated HLSL source
/// @param entry_points the list of entry points to validate
/// @return the result of the compile
Result HlslUsingFXC(const std::string& fxc_path,
                    const std::string& source,
                    const EntryPointList& entry_points);
#endif  // _WIN32

/// Msl attempts to compile the shader with the Metal Shader Compiler,
/// verifying that the shader compiles successfully.
/// @param xcrun_path path to xcrun
/// @param source the generated MSL source
/// @return the result of the compile
Result Msl(const std::string& xcrun_path, const std::string& source);

#ifdef TINT_ENABLE_MSL_VALIDATION_USING_METAL_API
/// Msl attempts to compile the shader with the runtime Metal Shader Compiler
/// API, verifying that the shader compiles successfully.
/// @param source the generated MSL source
/// @return the result of the compile
Result MslUsingMetalAPI(const std::string& source);
#endif  // TINT_ENABLE_MSL_VALIDATION_USING_METAL_API

}  // namespace tint::val

#endif  // SRC_TINT_VAL_VAL_H_
