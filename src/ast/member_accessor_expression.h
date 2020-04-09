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

#ifndef SRC_AST_MEMBER_ACCESSOR_EXPRESSION_H_
#define SRC_AST_MEMBER_ACCESSOR_EXPRESSION_H_

#include <memory>
#include <string>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/literal.h"

namespace tint {
namespace ast {

/// A member accessor expression
class MemberAccessorExpression : public Expression {
 public:
  /// Constructor
  MemberAccessorExpression();
  /// Constructor
  /// @param structure the structure
  /// @param member the member
  MemberAccessorExpression(std::unique_ptr<Expression> structure,
                           std::unique_ptr<IdentifierExpression> member);
  /// Constructor
  /// @param source the member accessor expression source
  /// @param structure the structure
  /// @param member the member
  MemberAccessorExpression(const Source& source,
                           std::unique_ptr<Expression> structure,
                           std::unique_ptr<IdentifierExpression> member);
  /// Move constructor
  MemberAccessorExpression(MemberAccessorExpression&&);
  ~MemberAccessorExpression() override;

  /// Sets the structure
  /// @param structure the structure
  void set_structure(std::unique_ptr<Expression> structure) {
    struct_ = std::move(structure);
  }
  /// @returns the structure
  Expression* structure() const { return struct_.get(); }

  /// Sets the member
  /// @param member the member
  void set_member(std::unique_ptr<IdentifierExpression> member) {
    member_ = std::move(member);
  }
  /// @returns the member expression
  IdentifierExpression* member() const { return member_.get(); }

  /// @returns true if this is a member accessor expression
  bool IsMemberAccessor() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  MemberAccessorExpression(const MemberAccessorExpression&) = delete;

  std::unique_ptr<Expression> struct_;
  std::unique_ptr<IdentifierExpression> member_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MEMBER_ACCESSOR_EXPRESSION_H_
