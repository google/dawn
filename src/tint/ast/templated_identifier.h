// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_AST_TEMPLATED_IDENTIFIER_H_
#define SRC_TINT_AST_TEMPLATED_IDENTIFIER_H_

#include "src/tint/ast/identifier.h"

// Forward declarations
namespace tint::ast {
class Attribute;
class Expression;
}  // namespace tint::ast

namespace tint::ast {

/// A templated identifier expression
class TemplatedIdentifier final : public utils::Castable<TemplatedIdentifier, Identifier> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param sym the symbol for the identifier
    /// @param args the template arguments
    /// @param attrs the identifier attributes
    TemplatedIdentifier(ProgramID pid,
                        NodeID nid,
                        const Source& src,
                        const Symbol& sym,
                        utils::VectorRef<const Expression*> args,
                        utils::VectorRef<const Attribute*> attrs);

    /// Destructor
    ~TemplatedIdentifier() override;

    /// Clones this node and all transitive child nodes using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const TemplatedIdentifier* Clone(CloneContext* ctx) const override;

    /// The templated arguments
    const utils::Vector<const Expression*, 3> arguments;

    /// Attributes on the identifier
    const utils::Vector<const Attribute*, 0> attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_TEMPLATED_IDENTIFIER_H_
