// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_BUILTIN_POLYFILL_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_BUILTIN_POLYFILL_H_

#include <string>

#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// Enumerator of polyfill levels.
enum class BuiltinPolyfillLevel {
    /// No polyfill needed, supported by the backend.
    kNone,
    /// Clamp or range check the parameters.
    kClampOrRangeCheck,
    /// Polyfill the entire function.
    kFull,
};

/// The set of polyfills that should be applied.
struct BuiltinPolyfillConfig {
    /// Should `clamp()` be polyfilled for integer values?
    bool clamp_int = false;
    /// Should `countLeadingZeros()` be polyfilled?
    bool count_leading_zeros = false;
    /// Should `countTrailingZeros()` be polyfilled?
    bool count_trailing_zeros = false;
    /// How should `extractBits()` be polyfilled?
    BuiltinPolyfillLevel extract_bits = BuiltinPolyfillLevel::kNone;
    /// Should `firstLeadingBit()` be polyfilled?
    bool first_leading_bit = false;
    /// Should `firstTrailingBit()` be polyfilled?
    bool first_trailing_bit = false;
    /// How should `insertBits()` be polyfilled?
    BuiltinPolyfillLevel insert_bits = BuiltinPolyfillLevel::kNone;
    /// Should `saturate()` be polyfilled?
    bool saturate = false;
    /// Should `textureSampleBaseClampToEdge()` be polyfilled for texture_2d<f32> textures?
    bool texture_sample_base_clamp_to_edge_2d_f32 = false;
};

/// BuiltinPolyfill is a transform that replaces calls to builtin functions and uses of other core
/// features with equivalent alternatives.
/// @param module the module to transform
/// @param config the polyfill configuration
/// @returns success or failure
Result<SuccessType> BuiltinPolyfill(Module& module, const BuiltinPolyfillConfig& config);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_BUILTIN_POLYFILL_H_
