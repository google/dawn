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

#ifndef SRC_TINT_AST_VARIABLE_H_
#define SRC_TINT_AST_VARIABLE_H_

#include <utility>
#include <vector>

#include "src/tint/ast/access.h"
#include "src/tint/ast/address_space.h"
#include "src/tint/ast/attribute.h"
#include "src/tint/ast/binding_attribute.h"
#include "src/tint/ast/expression.h"
#include "src/tint/ast/group_attribute.h"

// Forward declarations
namespace tint::ast {
class LocationAttribute;
class Type;
}  // namespace tint::ast

namespace tint::ast {

/// Variable is the base class for Var, Let, Const, Override and Parameter.
///
/// An instance of this class represents one of five constructs in WGSL: "var"  declaration, "let"
/// declaration, "override" declaration, "const" declaration, or formal parameter to a function.
///
/// @see https://www.w3.org/TR/WGSL/#value-decls
class Variable : public Castable<Variable, Node> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the variable source
    /// @param sym the variable symbol
    /// @param type the declared variable type
    /// @param constructor the constructor expression
    /// @param attributes the variable attributes
    Variable(ProgramID pid,
             NodeID nid,
             const Source& source,
             const Symbol& sym,
             const ast::Type* type,
             const Expression* constructor,
             utils::VectorRef<const Attribute*> attributes);

    /// Move constructor
    Variable(Variable&&);

    /// Destructor
    ~Variable() override;

    /// @returns true if the variable has both group and binding attributes
    bool HasBindingPoint() const {
        return ast::GetAttribute<ast::BindingAttribute>(attributes) != nullptr &&
               ast::GetAttribute<ast::GroupAttribute>(attributes) != nullptr;
    }

    /// @returns the kind of the variable, which can be used in diagnostics
    ///          e.g. "var", "let", "const", etc
    virtual const char* Kind() const = 0;

    /// The variable symbol
    const Symbol symbol;

    /// The declared variable type. This is null if the type is inferred, e.g.:
    ///   let f = 1.0;
    ///   var i = 1;
    const ast::Type* const type;

    /// The constructor expression or nullptr if none set
    const Expression* const constructor;

    /// The attributes attached to this variable
    const utils::Vector<const Attribute*, 2> attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_VARIABLE_H_
