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

#ifndef SRC_TINT_AST_MEMBER_ACCESSOR_EXPRESSION_H_
#define SRC_TINT_AST_MEMBER_ACCESSOR_EXPRESSION_H_

#include "src/tint/ast/identifier_expression.h"

namespace tint::ast {

/// A member accessor expression
class MemberAccessorExpression final : public Castable<MemberAccessorExpression, Expression> {
  public:
    /// Constructor
    /// @param program_id the identifier of the program that owns this node
    /// @param source the member accessor expression source
    /// @param structure the structure
    /// @param member the member
    MemberAccessorExpression(ProgramID program_id,
                             const Source& source,
                             const Expression* structure,
                             const IdentifierExpression* member);
    /// Move constructor
    MemberAccessorExpression(MemberAccessorExpression&&);
    ~MemberAccessorExpression() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const MemberAccessorExpression* Clone(CloneContext* ctx) const override;

    /// The structure
    const Expression* const structure;

    /// The member expression
    const IdentifierExpression* const member;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_MEMBER_ACCESSOR_EXPRESSION_H_
