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

#ifndef SRC_TINT_SEM_LOAD_H_
#define SRC_TINT_SEM_LOAD_H_

#include "src/tint/sem/value_expression.h"
#include "src/tint/type/reference.h"

namespace tint::sem {

/// Load is a semantic expression which represents the load of a reference to a non-reference value.
/// Loads from reference types are implicit in WGSL, so the Load semantic node shares the same AST
/// node as the inner semantic node.
class Load final : public utils::Castable<Load, ValueExpression> {
  public:
    /// Constructor
    /// @param reference the reference expression being loaded
    /// @param statement the statement that owns this expression
    Load(const ValueExpression* reference, const Statement* statement);

    /// Destructor
    ~Load() override;

    /// @return the reference being loaded
    const ValueExpression* Reference() const { return reference_; }

    /// @returns the type of the loaded reference.
    const type::Reference* ReferenceType() const {
        return static_cast<const type::Reference*>(reference_->Type());
    }

  private:
    ValueExpression const* const reference_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_LOAD_H_
