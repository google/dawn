// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_SEM_MATERIALIZE_H_
#define SRC_TINT_SEM_MATERIALIZE_H_

#include "src/tint/sem/expression.h"

namespace tint::sem {

/// Materialize is a semantic expression which represents the materialization of a value of an
/// abstract numeric type to a value of a concrete type.
/// Abstract numeric materialization is implicit in WGSL, so the Materialize semantic node shares
/// the same AST node as the inner semantic node.
/// Abstract numerics types may only be used by compile-time expressions, so a Materialize semantic
/// node must have a valid Constant value.
class Materialize final : public Castable<Materialize, Expression> {
  public:
    /// Constructor
    /// @param expr the inner expression, being materialized
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of this expression
    Materialize(const Expression* expr, const Statement* statement, const Constant* constant);

    /// Destructor
    ~Materialize() override;

    /// @return the target of the call
    const Expression* Expr() const { return expr_; }

  private:
    Expression const* const expr_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_MATERIALIZE_H_
