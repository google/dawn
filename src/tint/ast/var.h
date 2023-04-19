// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_AST_VAR_H_
#define SRC_TINT_AST_VAR_H_

#include <utility>
#include <vector>

#include "src/tint/ast/variable.h"

namespace tint::ast {

/// A "var" declaration is a name for typed storage.
///
/// Examples:
///
/// ```
///  // Declared outside a function, i.e. at module scope, requires
///  // a address space.
///  var<workgroup> width : i32;     // no initializer
///  var<private> height : i32 = 3;  // with initializer
///
///  // A variable declared inside a function doesn't take a address space,
///  // and maps to SPIR-V Function storage.
///  var computed_depth : i32;
///  var area : i32 = compute_area(width, height);
/// ```
///
/// @see https://www.w3.org/TR/WGSL/#var-decls
class Var final : public utils::Castable<Var, Variable> {
  public:
    /// Create a 'var' variable
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the variable source
    /// @param name the variable name
    /// @param type the declared variable type
    /// @param declared_address_space the declared address space
    /// @param declared_access the declared access control
    /// @param initializer the initializer expression
    /// @param attributes the variable attributes
    Var(ProgramID pid,
        NodeID nid,
        const Source& source,
        const Identifier* name,
        Type type,
        const Expression* declared_address_space,
        const Expression* declared_access,
        const Expression* initializer,
        utils::VectorRef<const Attribute*> attributes);

    /// Destructor
    ~Var() override;

    /// @returns "var"
    const char* Kind() const override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Var* Clone(CloneContext* ctx) const override;

    /// The declared address space
    const Expression* const declared_address_space = nullptr;

    /// The declared access control
    const Expression* const declared_access = nullptr;
};

/// A list of `var` declarations
using VarList = std::vector<const Var*>;

}  // namespace tint::ast

#endif  // SRC_TINT_AST_VAR_H_
