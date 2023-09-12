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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_ROBUSTNESS_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_ROBUSTNESS_H_

#include <string>
#include <unordered_set>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// Configuration options that control when to clamp accesses.
struct RobustnessConfig {
    /// Should non-pointer accesses be clamped?
    bool clamp_value = true;

    /// Should texture accesses be clamped?
    bool clamp_texture = true;

    /// Should accesses to pointers with the 'function' address space be clamped?
    bool clamp_function = true;
    /// Should accesses to pointers with the 'private' address space be clamped?
    bool clamp_private = true;
    /// Should accesses to pointers with the 'push_constant' address space be clamped?
    bool clamp_push_constant = true;
    /// Should accesses to pointers with the 'storage' address space be clamped?
    bool clamp_storage = true;
    /// Should accesses to pointers with the 'uniform' address space be clamped?
    bool clamp_uniform = true;
    /// Should accesses to pointers with the 'workgroup' address space be clamped?
    bool clamp_workgroup = true;

    /// Bindings that should always be ignored.
    std::unordered_set<tint::BindingPoint> bindings_ignored;

    /// Should the transform skip index clamping on runtime-sized arrays?
    bool disable_runtime_sized_array_index_clamping = false;
};

/// Robustness is a transform that prevents out-of-bounds memory accesses.
/// @param module the module to transform
/// @param config the robustness configuration
/// @returns an error string on failure
Result<SuccessType, std::string> Robustness(Module* module, const RobustnessConfig& config);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_ROBUSTNESS_H_
