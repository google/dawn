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

#ifndef SRC_TINT_AST_INTERNAL_ATTRIBUTE_H_
#define SRC_TINT_AST_INTERNAL_ATTRIBUTE_H_

#include <string>

#include "src/tint/ast/attribute.h"
#include "src/tint/utils/vector.h"

// Forward declarations
namespace tint::ast {
class IdentifierExpression;
}  // namespace tint::ast

namespace tint::ast {

/// An attribute used to indicate that a function is tint-internal.
/// These attributes are not produced by generators, but instead are usually
/// created by transforms for consumption by a particular backend.
class InternalAttribute : public utils::Castable<InternalAttribute, Attribute> {
  public:
    /// Constructor
    /// @param program_id the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param deps a list of identifiers that this attribute is dependent on
    InternalAttribute(ProgramID program_id,
                      NodeID nid,
                      utils::VectorRef<const IdentifierExpression*> deps);

    /// Destructor
    ~InternalAttribute() override;

    /// @return a short description of the internal attribute which will be
    /// displayed in WGSL as `@internal(<name>)` (but is not parsable).
    virtual std::string InternalName() const = 0;

    /// @returns the WGSL name for the attribute
    std::string Name() const override;

    /// A list of identifiers that this attribute is dependent on
    const utils::Vector<const IdentifierExpression*, 1> dependencies;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_INTERNAL_ATTRIBUTE_H_
