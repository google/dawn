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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_UNRESOLVED_IDENTIFIER_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_UNRESOLVED_IDENTIFIER_H_

#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/sem/expression.h"

namespace tint::resolver {

/// Represents an identifier that could not be resolved.
class UnresolvedIdentifier : public Castable<UnresolvedIdentifier, sem::Expression> {
  public:
    /// Constructor
    /// @param i the identifier that could not be resolved
    /// @param statement the statement that owns this expression
    UnresolvedIdentifier(const ast::IdentifierExpression* i, const sem::Statement* statement);

    /// Destructor
    ~UnresolvedIdentifier() override;

    /// @returns the identifier that could not be resolved
    const ast::IdentifierExpression* Identifier() const {
        return static_cast<const ast::IdentifierExpression*>(declaration_);
    }
};

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_UNRESOLVED_IDENTIFIER_H_
