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

#ifndef SRC_TINT_TRANSFORM_CLAMP_FRAG_DEPTH_H_
#define SRC_TINT_TRANSFORM_CLAMP_FRAG_DEPTH_H_

#include "src/tint/transform/transform.h"

// Forward declarations
namespace tint {
class CloneContext;
}  // namespace tint

namespace tint::transform {

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
class ClampFragDepth final : public utils::Castable<ClampFragDepth, Transform> {
  public:
    /// Constructor
    ClampFragDepth();
    /// Destructor
    ~ClampFragDepth() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_CLAMP_FRAG_DEPTH_H_
