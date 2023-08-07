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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_BUILTIN_POLYFILL_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_BUILTIN_POLYFILL_H_

#include <string>

#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::ir {
class Module;
}

namespace tint::ir::transform {

/// The set of polyfills that should be applied.
struct BuiltinPolyfillConfig {
    /// Should `countLeadingZeros()` be polyfilled?
    bool count_leading_zeros = false;
    /// Should `countTrailingZeros()` be polyfilled?
    bool count_trailing_zeros = false;
    /// Should `firstLeadingBit()` be polyfilled?
    bool first_leading_bit = false;
    /// Should `firstTrailingBit()` be polyfilled?
    bool first_trailing_bit = false;
    /// Should `saturate()` be polyfilled?
    bool saturate = false;
    /// Should `textureSampleBaseClampToEdge()` be polyfilled for texture_2d<f32> textures?
    bool texture_sample_base_clamp_to_edge_2d_f32 = false;
};

/// BuiltinPolyfill is a transform that replaces calls to builtin functions and uses of other core
/// features with equivalent alternatives.
/// @param module the module to transform
/// @param config the polyfill configuration
/// @returns an error string on failure
Result<SuccessType, std::string> BuiltinPolyfill(Module* module,
                                                 const BuiltinPolyfillConfig& config);

}  // namespace tint::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_BUILTIN_POLYFILL_H_
