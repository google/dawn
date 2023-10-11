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

#ifndef SRC_TINT_LANG_GLSL_VALIDATE_VALIDATE_H_
#define SRC_TINT_LANG_GLSL_VALIDATE_VALIDATE_H_

#include <string>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/pipeline_stage.h"
#include "src/tint/utils/result/result.h"

// Forward declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::glsl::validate {

using EntryPointList = std::vector<std::pair<std::string, ast::PipelineStage>>;

/// Validate checks that the GLSL source passes validation.
/// @param source the GLSL source
/// @param entry_points the list of entry points to validate
/// @return the result
Result<SuccessType> Validate(const std::string& source, const EntryPointList& entry_points);

}  // namespace tint::glsl::validate

#endif  // SRC_TINT_LANG_GLSL_VALIDATE_VALIDATE_H_
