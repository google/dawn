// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_CLAMP_FRAG_DEPTH_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_CLAMP_FRAG_DEPTH_H_

#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/reflection/reflection.h"

namespace tint::ast::transform {

/// Add clamping of the `@builtin(frag_depth)` output of fragment shaders using two push constants
/// provided by the outside environment. For example the following code:
///
/// ```
///   @fragment fn main() -> @builtin(frag_depth) f32 {
///       return 0.0;
///   }
/// ```
///
/// Is transformed to:
///
/// ```
///   enable chromium_experimental_push_constant;
///
///   struct FragDepthClampArgs {
///     min : f32,
///     max : f32,
///   }
///
///   var<push_constant> frag_depth_clamp_args : FragDepthClampArgs;
///
///   fn clamp_frag_depth(v : f32) -> f32 {
///     return clamp(v, frag_depth_clamp_args.min, frag_depth_clamp_args.max);
///   }
///
///   @fragment
///   fn main() -> @builtin(frag_depth) f32 {
///     return clamp_frag_depth(0.0);
///   }
/// ```
class ClampFragDepth final : public Castable<ClampFragDepth, ast::transform::Transform> {
  public:
    struct RangeOffsets {
        /// The offset of the min_depth push constant
        uint32_t min = 0;
        /// The offset of the max_depth push constant
        uint32_t max = 0;

        /// Reflection for this struct
        TINT_REFLECT(RangeOffsets, min, max);
    };

    /// Constructor
    ClampFragDepth();
    /// Destructor
    ~ClampFragDepth() override;

    /// Transform configuration options
    struct Config final : public Castable<Config, ast::transform::Data> {
        /// Constructor
        Config();

        /// Constructor
        /// @param off Offsets of the min_depth and max_depth push constants
        explicit Config(std::optional<RangeOffsets> off);

        /// Destructor
        ~Config() override;

        /// Offsets of the min_depth and max_depth push constants
        std::optional<RangeOffsets> offsets;

        /// Reflection for this struct
        TINT_REFLECT(Config, offsets);
    };

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_CLAMP_FRAG_DEPTH_H_
