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

#ifndef SRC_SEM_MEMBER_ACCESSOR_EXPRESSION_H_
#define SRC_SEM_MEMBER_ACCESSOR_EXPRESSION_H_

#include <vector>

#include "src/sem/expression.h"

namespace tint {

/// Forward declarations
namespace ast {
class MemberAccessorExpression;
}  // namespace ast

namespace sem {

/// Forward declarations
class Struct;
class StructMember;

/// MemberAccessorExpression holds the semantic information for a
/// ast::MemberAccessorExpression node.
class MemberAccessorExpression
    : public Castable<MemberAccessorExpression, Expression> {
 public:
  /// Constructor
  /// @param declaration the AST node
  /// @param type the resolved type of the expression
  /// @param statement the statement that owns this expression
  MemberAccessorExpression(ast::MemberAccessorExpression* declaration,
                           const sem::Type* type,
                           Statement* statement);

  /// Destructor
  ~MemberAccessorExpression() override;
};

/// StructMemberAccess holds the semantic information for a
/// ast::MemberAccessorExpression node that represents an access to a structure
/// member.
class StructMemberAccess
    : public Castable<StructMemberAccess, MemberAccessorExpression> {
 public:
  /// Constructor
  /// @param declaration the AST node
  /// @param type the resolved type of the expression
  /// @param statement the statement that owns this expression
  /// @param member the structure member
  StructMemberAccess(ast::MemberAccessorExpression* declaration,
                     const sem::Type* type,
                     Statement* statement,
                     const StructMember* member);

  /// Destructor
  ~StructMemberAccess() override;

  /// @returns the structure member
  StructMember const* Member() const { return member_; }

 private:
  StructMember const* const member_;
};

/// Swizzle holds the semantic information for a ast::MemberAccessorExpression
/// node that represents a vector swizzle.
class Swizzle : public Castable<Swizzle, MemberAccessorExpression> {
 public:
  /// Constructor
  /// @param declaration the AST node
  /// @param type the resolved type of the expression
  /// @param statement the statement that
  /// @param indices the swizzle indices
  Swizzle(ast::MemberAccessorExpression* declaration,
          const sem::Type* type,
          Statement* statement,
          std::vector<uint32_t> indices);

  /// Destructor
  ~Swizzle() override;

  /// @return the swizzle indices, if this is a vector swizzle
  const std::vector<uint32_t>& Indices() const { return indices_; }

 private:
  std::vector<uint32_t> const indices_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_MEMBER_ACCESSOR_EXPRESSION_H_
