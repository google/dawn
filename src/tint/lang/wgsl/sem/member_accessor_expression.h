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

#ifndef SRC_TINT_LANG_WGSL_SEM_MEMBER_ACCESSOR_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_SEM_MEMBER_ACCESSOR_EXPRESSION_H_

#include "src/tint/lang/wgsl/sem/accessor_expression.h"
#include "src/tint/utils/containers/vector.h"

// Forward declarations
namespace tint::ast {
class MemberAccessorExpression;
}  // namespace tint::ast
namespace tint::type {
class StructMember;
}  // namespace tint::type

namespace tint::sem {

/// MemberAccessorExpression is the base class for all semantic information for a
/// ast::MemberAccessorExpression node.
class MemberAccessorExpression : public Castable<MemberAccessorExpression, AccessorExpression> {
  public:
    /// Destructor
    ~MemberAccessorExpression() override;

  protected:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param stage the earliest evaluation stage for the expression
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null.
    /// @param object the object that holds the member being accessed
    /// @param has_side_effects whether this expression may have side effects
    /// @param root_ident the (optional) root identifier for this expression
    MemberAccessorExpression(const ast::MemberAccessorExpression* declaration,
                             const type::Type* type,
                             core::EvaluationStage stage,
                             const Statement* statement,
                             const constant::Value* constant,
                             const ValueExpression* object,
                             bool has_side_effects,
                             const Variable* root_ident = nullptr);
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
    /// @param constant the constant value of the expression. May be null
    /// @param object the object that holds the member being accessed
    /// @param member the structure member
    /// @param has_side_effects whether this expression may have side effects
    /// @param root_ident the (optional) root identifier for this expression
    StructMemberAccess(const ast::MemberAccessorExpression* declaration,
                       const type::Type* type,
                       const Statement* statement,
                       const constant::Value* constant,
                       const ValueExpression* object,
                       const type::StructMember* member,
                       bool has_side_effects,
                       const Variable* root_ident = nullptr);

    /// Destructor
    ~StructMemberAccess() override;

    /// @returns the structure member
    type::StructMember const* Member() const { return member_; }

  private:
    type::StructMember const* const member_;
};

/// Swizzle holds the semantic information for a ast::MemberAccessorExpression
/// node that represents a vector swizzle.
class Swizzle final : public Castable<Swizzle, MemberAccessorExpression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param object the object that holds the member being accessed
    /// @param indices the swizzle indices
    /// @param has_side_effects whether this expression may have side effects
    /// @param root_ident the (optional) root identifier for this expression
    Swizzle(const ast::MemberAccessorExpression* declaration,
            const type::Type* type,
            const Statement* statement,
            const constant::Value* constant,
            const ValueExpression* object,
            VectorRef<uint32_t> indices,
            bool has_side_effects,
            const Variable* root_ident = nullptr);

    /// Destructor
    ~Swizzle() override;

    /// @return the swizzle indices, if this is a vector swizzle
    const auto& Indices() const { return indices_; }

  private:
    tint::Vector<uint32_t, 4> const indices_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_MEMBER_ACCESSOR_EXPRESSION_H_
