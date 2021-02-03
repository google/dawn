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

#include "src/semantic/expression.h"

namespace tint {
namespace semantic {

/// MemberAccessorExpression holds the semantic information for a
/// ast::MemberAccessorExpression node.
class MemberAccessorExpression
    : public Castable<MemberAccessorExpression, Expression> {
 public:
  /// Constructor
  /// @param type the resolved type of the expression
  /// @param is_swizzle true if this member access is for a vector swizzle
  MemberAccessorExpression(type::Type* type, bool is_swizzle);

  /// @return true if this member access is for a vector swizzle
  bool IsSwizzle() const { return is_swizzle_; }

 private:
  bool const is_swizzle_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_MEMBER_ACCESSOR_EXPRESSION_H_
