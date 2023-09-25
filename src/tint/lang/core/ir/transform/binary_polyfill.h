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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_BINARY_POLYFILL_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_BINARY_POLYFILL_H_

#include <string>

#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// The set of polyfills that should be applied.
struct BinaryPolyfillConfig {
    /// Should the RHS of a shift be masked to make it modulo the bit-width of the LHS?
    bool bitshift_modulo = false;
    /// Should integer divide and modulo be polyfilled to avoid DBZ and integer overflow?
    bool int_div_mod = false;
};

/// BinaryPolyfill is a transform that modifies binary instructions to prepare them for raising to
/// backend dialects that may have different semantics.
/// @param module the module to transform
/// @param config the polyfill configuration
/// @returns success or failure
Result<SuccessType> BinaryPolyfill(Module& module, const BinaryPolyfillConfig& config);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_BINARY_POLYFILL_H_
