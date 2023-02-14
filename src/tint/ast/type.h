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

#ifndef SRC_TINT_AST_TYPE_H_
#define SRC_TINT_AST_TYPE_H_

#include "src/tint/program_id.h"

// Forward declarations
namespace tint::ast {
class IdentifierExpression;
}  // namespace tint::ast

namespace tint::ast {

/// Type is a thin wrapper around an IdentifierExpression, to help statically disambiguate known
/// type expressions from other expressions.
struct Type {
    /// The type expression
    const IdentifierExpression* expr = nullptr;

    /// Indirection operator for accessing the type's expression
    /// @return #expr
    const IdentifierExpression* operator->() const { return expr; }

    /// Implicit conversion operator to the type's expression
    /// @return #expr
    operator const IdentifierExpression*() const { return expr; }
};

}  // namespace tint::ast

namespace tint {

/// @param type an AST type
/// @returns the ProgramID of the given AST type.
ProgramID ProgramIDOf(ast::Type type);

}  // namespace tint

#endif  // SRC_TINT_AST_TYPE_H_
