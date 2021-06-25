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

#ifndef SRC_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_
#define SRC_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// CanonicalizeEntryPointIO is a transform used to rewrite shader entry point
/// interfaces into a form that the generators can handle. After the transform,
/// an entry point's parameters will be aggregated into a single struct, and its
/// return type will either be a struct or void. All structs in the module that
/// have entry point IO decorations will have exactly one pipeline stage usage.
///
/// Before:
/// ```
/// struct Locations{
///   [[location(1)]] loc1 : f32;
///   [[location(2)]] loc2 : vec4<u32>;
/// };
///
/// [[stage(fragment)]]
/// fn frag_main([[builtin(position)]] coord : vec4<f32>,
///              locations : Locations) -> [[location(0)]] f32 {
///   var col : f32 = (coord.x * locations.loc1);
///   return col;
/// }
/// ```
///
/// After:
/// ```
/// struct Locations{
///   loc1 : f32;
///   loc2 : vec4<u32>;
/// };
///
/// struct frag_main_in {
///   [[builtin(position)]] coord : vec4<f32>;
///   [[location(1)]] loc1 : f32;
///   [[location(2)]] loc2 : vec4<u32>
/// };
///
/// struct frag_main_out {
///   [[location(0)]] loc0 : f32;
/// };
///
/// [[stage(fragment)]]
/// fn frag_main(in : frag_main_in) -> frag_main_out {
///   const coord = in.coord;
///   const locations = Locations(in.loc1, in.loc2);
///   var col : f32 = (coord.x * locations.loc1);
///   var retval : frag_main_out;
///   retval.loc0 = col;
///   return retval;
/// }
/// ```
class CanonicalizeEntryPointIO
    : public Castable<CanonicalizeEntryPointIO, Transform> {
 public:
  /// BuiltinStyle is an enumerator of different ways to emit builtins.
  enum class BuiltinStyle {
    /// Use non-struct function parameters for all builtins.
    kParameter,
    /// Use struct members for all builtins.
    kStructMember,
  };

  /// Configuration options for the transform.
  struct Config : public Castable<Config, Data> {
    /// Constructor
    /// @param builtins the approach to use for emitting builtins.
    /// @param sample_mask an optional sample mask to combine with shader masks
    explicit Config(BuiltinStyle builtins, uint32_t sample_mask = 0xFFFFFFFF);

    /// Copy constructor
    Config(const Config&);

    /// Destructor
    ~Config() override;

    /// The approach to use for emitting builtins.
    BuiltinStyle const builtin_style;

    /// A fixed sample mask to combine into masks produced by fragment shaders.
    uint32_t const fixed_sample_mask;
  };

  /// Constructor
  CanonicalizeEntryPointIO();
  ~CanonicalizeEntryPointIO() override;

 protected:
  /// Runs the transform using the CloneContext built for transforming a
  /// program. Run() is responsible for calling Clone() on the CloneContext.
  /// @param ctx the CloneContext primed with the input program and
  /// ProgramBuilder
  /// @param inputs optional extra transform-specific input data
  /// @param outputs optional extra transform-specific output data
  void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_
