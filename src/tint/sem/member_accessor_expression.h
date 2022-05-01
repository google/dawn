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

#ifndef SRC_TINT_SEM_MEMBER_ACCESSOR_EXPRESSION_H_
#define SRC_TINT_SEM_MEMBER_ACCESSOR_EXPRESSION_H_

#include <vector>

#include "src/tint/sem/expression.h"

// Forward declarations
namespace tint::ast {
class MemberAccessorExpression;
}  // namespace tint::ast
namespace tint::sem {
class Struct;
class StructMember;
}  // namespace tint::sem

namespace tint::sem {

/// MemberAccessorExpression holds the semantic information for a
/// ast::MemberAccessorExpression node.
class MemberAccessorExpression : public Castable<MemberAccessorExpression, Expression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param statement the statement that owns this expression
    /// @param has_side_effects whether this expression may have side effects
    /// @param source_var the (optional) source variable for this expression
    MemberAccessorExpression(const ast::MemberAccessorExpression* declaration,
                             const sem::Type* type,
                             const Statement* statement,
                             bool has_side_effects,
                             const Variable* source_var = nullptr);

    /// Destructor
    ~MemberAccessorExpression() override;
};

/// StructMemberAccess holds the semantic information for a
/// ast::MemberAccessorExpression node that represents an access to a structure
/// member.
class StructMemberAccess final : public Castable<StructMemberAccess, MemberAccessorExpression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param statement the statement that owns this expression
    /// @param member the structure member
    /// @param has_side_effects whether this expression may have side effects
    /// @param source_var the (optional) source variable for this expression
    StructMemberAccess(const ast::MemberAccessorExpression* declaration,
                       const sem::Type* type,
                       const Statement* statement,
                       const StructMember* member,
                       bool has_side_effects,
                       const Variable* source_var = nullptr);

    /// Destructor
    ~StructMemberAccess() override;

    /// @returns the structure member
    StructMember const* Member() const { return member_; }

  private:
    StructMember const* const member_;
};

/// Swizzle holds the semantic information for a ast::MemberAccessorExpression
/// node that represents a vector swizzle.
class Swizzle final : public Castable<Swizzle, MemberAccessorExpression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param statement the statement that owns this expression
    /// @param indices the swizzle indices
    /// @param has_side_effects whether this expression may have side effects
    /// @param source_var the (optional) source variable for this expression
    Swizzle(const ast::MemberAccessorExpression* declaration,
            const sem::Type* type,
            const Statement* statement,
            std::vector<uint32_t> indices,
            bool has_side_effects,
            const Variable* source_var = nullptr);

    /// Destructor
    ~Swizzle() override;

    /// @return the swizzle indices, if this is a vector swizzle
    const std::vector<uint32_t>& Indices() const { return indices_; }

  private:
    std::vector<uint32_t> const indices_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_MEMBER_ACCESSOR_EXPRESSION_H_
