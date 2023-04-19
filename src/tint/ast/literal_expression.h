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

#ifndef SRC_TINT_AST_LITERAL_EXPRESSION_H_
#define SRC_TINT_AST_LITERAL_EXPRESSION_H_

#include <string>

#include "src/tint/ast/expression.h"

namespace tint::ast {

/// Base class for a literal value expressions
class LiteralExpression : public utils::Castable<LiteralExpression, Expression> {
  public:
    ~LiteralExpression() override;

  protected:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the input source
    LiteralExpression(ProgramID pid, NodeID nid, const Source& src);
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_LITERAL_EXPRESSION_H_
