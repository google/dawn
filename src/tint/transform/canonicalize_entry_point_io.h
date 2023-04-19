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

#ifndef SRC_TINT_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_
#define SRC_TINT_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// CanonicalizeEntryPointIO is a transform used to rewrite shader entry point
/// interfaces into a form that the generators can handle. Each entry point
/// function is stripped of all shader IO attributes and wrapped in a function
/// that provides the shader interface.
/// The transform config determines whether to use global variables, structures,
/// or parameters for the shader inputs and outputs, and optionally adds
/// additional builtins to the shader interface.
///
/// Before:
/// ```
/// struct Locations{
///   @location(1) loc1 : f32;
///   @location(2) loc2 : vec4<u32>;
/// };
///
/// @fragment
/// fn frag_main(@builtin(position) coord : vec4<f32>,
///              locations : Locations) -> @location(0) f32 {
///   if (coord.w > 1.0) {
///     return 0.0;
///   }
///   var col : f32 = (coord.x * locations.loc1);
///   return col;
/// }
/// ```
///
/// After (using structures for all parameters):
/// ```
/// struct Locations{
///   loc1 : f32;
///   loc2 : vec4<u32>;
/// };
///
/// struct frag_main_in {
///   @builtin(position) coord : vec4<f32>;
///   @location(1) loc1 : f32;
///   @location(2) loc2 : vec4<u32>
/// };
///
/// struct frag_main_out {
///   @location(0) loc0 : f32;
/// };
///
/// fn frag_main_inner(coord : vec4<f32>,
///                    locations : Locations) -> f32 {
///   if (coord.w > 1.0) {
///     return 0.0;
///   }
///   var col : f32 = (coord.x * locations.loc1);
///   return col;
/// }
///
/// @fragment
/// fn frag_main(in : frag_main_in) -> frag_main_out {
///   let inner_retval = frag_main_inner(in.coord, Locations(in.loc1, in.loc2));
///   var wrapper_result : frag_main_out;
///   wrapper_result.loc0 = inner_retval;
///   return wrapper_result;
/// }
/// ```
///
/// @note Depends on the following transforms to have been run first:
/// * Unshadow
class CanonicalizeEntryPointIO final : public utils::Castable<CanonicalizeEntryPointIO, Transform> {
  public:
    /// ShaderStyle is an enumerator of different ways to emit shader IO.
    enum class ShaderStyle {
        /// Target SPIR-V (using global variables).
        kSpirv,
        /// Target GLSL (using global variables).
        kGlsl,
        /// Target MSL (using non-struct function parameters for builtins).
        kMsl,
        /// Target HLSL (using structures for all IO).
        kHlsl,
    };

    /// Configuration options for the transform.
    struct Config final : public utils::Castable<Config, Data> {
        /// Constructor
        /// @param style the approach to use for emitting shader IO.
        /// @param sample_mask an optional sample mask to combine with shader masks
        /// @param emit_vertex_point_size `true` to generate a pointsize builtin
        explicit Config(ShaderStyle style,
                        uint32_t sample_mask = 0xFFFFFFFF,
                        bool emit_vertex_point_size = false);

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// The approach to use for emitting shader IO.
        const ShaderStyle shader_style;

        /// A fixed sample mask to combine into masks produced by fragment shaders.
        const uint32_t fixed_sample_mask;

        /// Set to `true` to generate a pointsize builtin and have it set to 1.0
        /// from all vertex shaders in the module.
        const bool emit_vertex_point_size;
    };

    /// Constructor
    CanonicalizeEntryPointIO();
    ~CanonicalizeEntryPointIO() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_
