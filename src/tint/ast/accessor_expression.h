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

#ifndef SRC_TINT_AST_ACCESSOR_EXPRESSION_H_
#define SRC_TINT_AST_ACCESSOR_EXPRESSION_H_

#include "src/tint/ast/expression.h"

namespace tint::ast {

/// Base class for IndexAccessorExpression and MemberAccessorExpression
class AccessorExpression : public utils::Castable<AccessorExpression, Expression> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the member accessor expression source
    /// @param object the object
    AccessorExpression(ProgramID pid, NodeID nid, const Source& source, const Expression* object);

    /// Destructor
    ~AccessorExpression() override;

    /// The object being accessed
    const Expression* const object;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_ACCESSOR_EXPRESSION_H_
