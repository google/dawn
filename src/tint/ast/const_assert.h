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

#ifndef SRC_TINT_AST_CONST_ASSERT_H_
#define SRC_TINT_AST_CONST_ASSERT_H_

#include "src/tint/ast/statement.h"
#include "src/tint/ast/variable.h"

namespace tint::ast {

/// A `const_assert` statement
class ConstAssert final : public utils::Castable<ConstAssert, Statement> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the variable statement source
    /// @param condition the assertion condition
    ConstAssert(ProgramID pid, NodeID nid, const Source& source, const Expression* condition);

    /// Destructor
    ~ConstAssert() override;

    /// Clones this node and all transitive child nodes using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const ConstAssert* Clone(CloneContext* ctx) const override;

    /// The assertion condition
    const Expression* const condition;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_CONST_ASSERT_H_
