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

#ifndef SRC_TINT_TRANSFORM_SPIRV_ATOMIC_H_
#define SRC_TINT_TRANSFORM_SPIRV_ATOMIC_H_

#include <string>

#include "src/tint/ast/internal_attribute.h"
#include "src/tint/builtin/function.h"
#include "src/tint/transform/transform.h"

// Forward declarations
namespace tint {
class CloneContext;
}  // namespace tint

namespace tint::transform {

/// SpirvAtomic is a transform that replaces calls to stub functions created by the SPIR-V reader
/// with calls to the WGSL atomic builtin. It also makes sure to replace variable declarations that
/// are the target of the atomic operations with an atomic declaration of the same type. For
/// structs, it creates a copy of the original struct with atomic members.
class SpirvAtomic final : public utils::Castable<SpirvAtomic, Transform> {
  public:
    /// Constructor
    SpirvAtomic();
    /// Destructor
    ~SpirvAtomic() override;

    /// Stub is an attribute applied to stub SPIR-V reader generated functions that need to be
    /// translated to an atomic builtin.
    class Stub final : public utils::Castable<Stub, ast::InternalAttribute> {
      public:
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        /// @param builtin the atomic builtin this stub represents
        Stub(ProgramID pid, ast::NodeID nid, builtin::Function builtin);
        /// Destructor
        ~Stub() override;

        /// @return a short description of the internal attribute which will be
        /// displayed as `@internal(<name>)`
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const Stub* Clone(CloneContext* ctx) const override;

        /// The type of the intrinsic
        const builtin::Function builtin;
    };

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_SPIRV_ATOMIC_H_
