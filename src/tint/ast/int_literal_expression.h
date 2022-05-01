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

#ifndef SRC_TINT_AST_INT_LITERAL_EXPRESSION_H_
#define SRC_TINT_AST_INT_LITERAL_EXPRESSION_H_

#include "src/tint/ast/literal_expression.h"

namespace tint::ast {

/// An integer literal. This could be either signed or unsigned.
class IntLiteralExpression : public Castable<IntLiteralExpression, LiteralExpression> {
  public:
    ~IntLiteralExpression() override;

    /// @returns the literal value as a u32
    virtual uint32_t ValueAsU32() const = 0;

    /// @returns the literal value as an i32
    int32_t ValueAsI32() const { return static_cast<int32_t>(ValueAsU32()); }

  protected:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node
    IntLiteralExpression(ProgramID pid, const Source& src);
};  // namespace ast

}  // namespace tint::ast

#endif  // SRC_TINT_AST_INT_LITERAL_EXPRESSION_H_
