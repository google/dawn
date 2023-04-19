// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_AST_RETURN_STATEMENT_H_
#define SRC_TINT_AST_RETURN_STATEMENT_H_

#include "src/tint/ast/expression.h"
#include "src/tint/ast/statement.h"

namespace tint::ast {

/// A return statement
class ReturnStatement final : public utils::Castable<ReturnStatement, Statement> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    ReturnStatement(ProgramID pid, NodeID nid, const Source& src);

    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param value the return value
    ReturnStatement(ProgramID pid, NodeID nid, const Source& src, const Expression* value);

    /// Destructor
    ~ReturnStatement() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const ReturnStatement* Clone(CloneContext* ctx) const override;

    /// The value returned. May be null.
    const Expression* const value;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_RETURN_STATEMENT_H_
