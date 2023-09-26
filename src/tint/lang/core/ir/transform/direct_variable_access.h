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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_

#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// DirectVariableAccessOptions adjusts the behaviour of the transform.
struct DirectVariableAccessOptions {
    /// If true, then 'private' sub-object pointer arguments will be transformed.
    bool transform_private = false;
    /// If true, then 'function' sub-object pointer arguments will be transformed.
    bool transform_function = false;
};

/// DirectVariableAccess is a transform that transforms pointer parameters in the 'storage',
/// 'uniform' and 'workgroup' address space so that they're accessed directly by the function,
/// instead of being passed by pointer.
///
/// DirectVariableAccess works by creating specializations of functions that have pointer
/// parameters, one specialization for each pointer argument's unique access chain 'shape' from a
/// unique variable. Calls to specialized functions are transformed so that the pointer arguments
/// are replaced with an array of access-chain indicies, and if the pointer is in the 'function' or
/// 'private' address space, also with a pointer to the root object. For more information, see the
/// comments in src/tint/lang/wgsl/ast/transform/direct_variable_access.cc.
///
/// @param module the module to transform
/// @returns error diagnostics on failure
Result<SuccessType> DirectVariableAccess(Module& module,
                                         const DirectVariableAccessOptions& options);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_
