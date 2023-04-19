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

#ifndef SRC_TINT_AST_IDENTIFIER_H_
#define SRC_TINT_AST_IDENTIFIER_H_

#include "src/tint/ast/node.h"

namespace tint::ast {

/// An identifier
class Identifier : public utils::Castable<Identifier, Node> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param sym the symbol for the identifier
    Identifier(ProgramID pid, NodeID nid, const Source& src, Symbol sym);

    /// Destructor
    ~Identifier() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Identifier* Clone(CloneContext* ctx) const override;

    /// The symbol for the identifier
    const Symbol symbol;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_IDENTIFIER_H_
