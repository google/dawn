// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_TRANSFORM_BUILTIN_POLYFILL_H_
#define SRC_TINT_TRANSFORM_BUILTIN_POLYFILL_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Implements builtins for backends that do not have a native implementation.
class BuiltinPolyfill final : public utils::Castable<BuiltinPolyfill, Transform> {
  public:
    /// Constructor
    BuiltinPolyfill();
    /// Destructor
    ~BuiltinPolyfill() override;

    /// Enumerator of polyfill levels
    enum class Level {
        /// No polyfill needed, supported by the backend.
        kNone,
        /// Clamp the parameters to the inner implementation.
        kClampParameters,
        /// Range check the input.
        kRangeCheck,
        /// Polyfill the entire function
        kFull,
    };

    /// Specifies the builtins that should be polyfilled by the transform.
    struct Builtins {
        /// What level should `acosh` be polyfilled?
        Level acosh = Level::kNone;
        /// Should `asinh` be polyfilled?
        bool asinh = false;
        /// What level should `atanh` be polyfilled?
        Level atanh = Level::kNone;
        /// Should storage textures of format 'bgra8unorm' be replaced with 'rgba8unorm'?
        bool bgra8unorm = false;
        /// Should the RHS of `<<` and `>>` be wrapped in a modulo bit-width of LHS?
        bool bitshift_modulo = false;
        /// Should `clamp()` be polyfilled for integer values (scalar or vector)?
        bool clamp_int = false;
        /// Should `countLeadingZeros()` be polyfilled?
        bool count_leading_zeros = false;
        /// Should `countTrailingZeros()` be polyfilled?
        bool count_trailing_zeros = false;
        /// Should converting f32 to i32 or u32 be polyfilled?
        bool conv_f32_to_iu32 = false;
        /// What level should `extractBits()` be polyfilled?
        Level extract_bits = Level::kNone;
        /// Should `firstLeadingBit()` be polyfilled?
        bool first_leading_bit = false;
        /// Should `firstTrailingBit()` be polyfilled?
        bool first_trailing_bit = false;
        /// Should `insertBits()` be polyfilled?
        Level insert_bits = Level::kNone;
        /// Should integer scalar / vector divides and modulos be polyfilled to avoid DBZ and
        /// integer overflows?
        bool int_div_mod = false;
        /// Should float modulos be polyfilled to emit a precise modulo operation as per the spec?
        bool precise_float_mod = false;
        /// Should `reflect()` be polyfilled for vec2<f32>?
        bool reflect_vec2_f32 = false;
        /// Should `saturate()` be polyfilled?
        bool saturate = false;
        /// Should `sign()` be polyfilled for integer types?
        bool sign_int = false;
        /// Should `textureSampleBaseClampToEdge()` be polyfilled for texture_2d<f32> textures?
        bool texture_sample_base_clamp_to_edge_2d_f32 = false;
        /// Should the vector form of `quantizeToF16()` be polyfilled with a scalar implementation?
        /// See crbug.com/tint/1741
        bool quantize_to_vec_f16 = false;
        /// Should `workgroupUniformLoad()` be polyfilled?
        bool workgroup_uniform_load = false;
    };

    /// Config is consumed by the BuiltinPolyfill transform.
    /// Config specifies the builtins that should be polyfilled.
    struct Config final : public utils::Castable<Data, transform::Data> {
        /// Constructor
        /// @param b the list of builtins to polyfill
        explicit Config(const Builtins& b);

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// The builtins to polyfill
        const Builtins builtins;
    };

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_BUILTIN_POLYFILL_H_
