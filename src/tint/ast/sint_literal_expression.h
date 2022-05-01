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

#ifndef SRC_TINT_AST_SINT_LITERAL_EXPRESSION_H_
#define SRC_TINT_AST_SINT_LITERAL_EXPRESSION_H_

#include <string>

#include "src/tint/ast/int_literal_expression.h"

namespace tint::ast {

/// A signed int literal
class SintLiteralExpression final : public Castable<SintLiteralExpression, IntLiteralExpression> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node
    /// @param value the signed int literals value
    SintLiteralExpression(ProgramID pid, const Source& src, int32_t value);
    ~SintLiteralExpression() override;

    /// @returns the literal value as a u32
    uint32_t ValueAsU32() const override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const SintLiteralExpression* Clone(CloneContext* ctx) const override;

    /// The int literal value
    const int32_t value;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_SINT_LITERAL_EXPRESSION_H_
