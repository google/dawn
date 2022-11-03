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

#include <string>

#include "src/tint/ast/internal_attribute.h"
#include "src/tint/transform/transform.h"

namespace tint::transform {

/// A transform to be used by the MSL backend which will:
/// * Apply the `@internal('packed_vector')` attribute (PackedVec3::Attribute) to all host-sharable
///   structure members that have a vec3<T> type.
/// * Cast all direct (not sub-accessed) loads of these packed vectors to the 'unpacked' vec3<T>
///   type before usage.
///
/// This transform papers over overload holes in the MSL standard library where an MSL
/// `packed_vector` type cannot be interchangable used as a regular `vec` type.
class PackedVec3 final : public Castable<PackedVec3, Transform> {
  public:
    /// Attribute is the attribute applied to padded vector structure members.
    class Attribute final : public Castable<Attribute, ast::InternalAttribute> {
      public:
        /// Constructor
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        Attribute(ProgramID pid, ast::NodeID nid);
        /// Destructor
        ~Attribute() override;

        /// @returns "packed_vector".
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const Attribute* Clone(CloneContext* ctx) const override;
    };

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
