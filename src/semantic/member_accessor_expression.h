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

#ifndef SRC_SEMANTIC_MEMBER_ACCESSOR_EXPRESSION_H_
#define SRC_SEMANTIC_MEMBER_ACCESSOR_EXPRESSION_H_

#include <vector>

#include "src/semantic/expression.h"

namespace tint {
namespace semantic {

/// MemberAccessorExpression holds the semantic information for a
/// ast::MemberAccessorExpression node.
class MemberAccessorExpression
    : public Castable<MemberAccessorExpression, Expression> {
 public:
  /// Constructor
  /// @param declaration the AST node
  /// @param type the resolved type of the expression
  /// @param statement the statement that owns this expression
  /// @param swizzle if this member access is for a vector swizzle, the swizzle
  /// indices
  MemberAccessorExpression(ast::Expression* declaration,
                           type::Type* type,
                           Statement* statement,
                           std::vector<uint32_t> swizzle);

  /// Destructor
  ~MemberAccessorExpression() override;

  /// @return true if this member access is for a vector swizzle
  bool IsSwizzle() const { return !swizzle_.empty(); }

  /// @return the swizzle indices, if this is a vector swizzle
  const std::vector<uint32_t>& Swizzle() const { return swizzle_; }

 private:
  std::vector<uint32_t> const swizzle_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_MEMBER_ACCESSOR_EXPRESSION_H_
