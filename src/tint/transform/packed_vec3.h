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

#ifndef SRC_TINT_TRANSFORM_PACKED_VEC3_H_
#define SRC_TINT_TRANSFORM_PACKED_VEC3_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// A transform to be used by the MSL backend which will:
/// * Replace `vec3<T>` types with an internal `__packed_vec3` type when they are used in
///   host-shareable address spaces.
/// * Wrap generated `__packed_vec3` types in a structure when they are used in arrays, so that we
///   ensure that the array has the correct element stride.
/// * Multi-version structures that contain `vec3<T>` types when they are used in host-shareable
///   memory, to avoid modifying uses in other address spaces.
/// * Rewrite matrix types that have three rows into arrays of column vectors.
/// * Insert calls to helper functions to convert expressions that use these types to or from the
///   regular vec3 types when accessing host-shareable memory.
/// * Cast all direct (not sub-accessed) loads of these packed vectors to the 'unpacked' vec3<T>
///   type before usage.
///
/// This transform is necessary in order to emit vec3 types with the correct size (so that scalars
/// can follow them in structures), and also to ensure that padding bytes are preserved when writing
/// to a vec3, an array of vec3 elements, or a matrix with vec3 column type.
///
/// @note Depends on the following transforms to have been run first:
/// * ExpandCompoundAssignment
class PackedVec3 final : public utils::Castable<PackedVec3, Transform> {
  public:
    /// Constructor
    PackedVec3();
    /// Destructor
    ~PackedVec3() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_PACKED_VEC3_H_
