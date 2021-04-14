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
class CanonicalizeEntryPointIO : public Transform {
 public:
  /// Constructor
  CanonicalizeEntryPointIO();
  ~CanonicalizeEntryPointIO() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific input data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_CANONICALIZE_ENTRY_POINT_IO_H_
