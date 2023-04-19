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

#ifndef SRC_TINT_TRANSFORM_ADD_BLOCK_ATTRIBUTE_H_
#define SRC_TINT_TRANSFORM_ADD_BLOCK_ATTRIBUTE_H_

#include <string>

#include "src/tint/ast/internal_attribute.h"
#include "src/tint/transform/transform.h"

namespace tint::transform {

/// AddBlockAttribute is a transform that wrap the store type of a buffer into a struct if possible,
/// then adds an `@internal(block)` attribute to the wrapper struct.
class AddBlockAttribute final : public utils::Castable<AddBlockAttribute, Transform> {
  public:
    /// BlockAttribute is an InternalAttribute that is used to decorate a
    // structure that is used as a buffer in SPIR-V or GLSL.
    class BlockAttribute final : public utils::Castable<BlockAttribute, ast::InternalAttribute> {
      public:
        /// Constructor
        /// @param program_id the identifier of the program that owns this node
        /// @param nid the unique node identifier
        BlockAttribute(ProgramID program_id, ast::NodeID nid);
        /// Destructor
        ~BlockAttribute() override;

        /// @return a short description of the internal attribute which will be
        /// displayed as `@internal(<name>)`
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const BlockAttribute* Clone(CloneContext* ctx) const override;
    };

    /// Constructor
    AddBlockAttribute();

    /// Destructor
    ~AddBlockAttribute() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_ADD_BLOCK_ATTRIBUTE_H_
