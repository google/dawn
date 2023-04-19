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

#include "src/tint/ast/attribute.h"
#include "src/tint/ast/binding_attribute.h"
#include "src/tint/ast/expression.h"
#include "src/tint/ast/group_attribute.h"
#include "src/tint/ast/type.h"
#include "src/tint/builtin/access.h"
#include "src/tint/builtin/address_space.h"

// Forward declarations
namespace tint::ast {
class Identifier;
class LocationAttribute;
}  // namespace tint::ast

namespace tint::ast {

/// Variable is the base class for Var, Let, Const, Override and Parameter.
///
/// An instance of this class represents one of five constructs in WGSL: "var"  declaration, "let"
/// declaration, "override" declaration, "const" declaration, or formal parameter to a function.
///
/// @see https://www.w3.org/TR/WGSL/#value-decls
class Variable : public utils::Castable<Variable, Node> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the variable source
    /// @param name The struct member name
    /// @param type the declared variable type
    /// @param initializer the initializer expression
    /// @param attributes the variable attributes
    Variable(ProgramID pid,
             NodeID nid,
             const Source& src,
             const Identifier* name,
             Type type,
             const Expression* initializer,
             utils::VectorRef<const Attribute*> attributes);

    /// Destructor
    ~Variable() override;

    /// @returns true if the variable has both group and binding attributes
    bool HasBindingPoint() const {
        return HasAttribute<BindingAttribute>(attributes) &&
               HasAttribute<GroupAttribute>(attributes);
    }

    /// @returns the kind of the variable, which can be used in diagnostics
    ///          e.g. "var", "let", "const", etc
    virtual const char* Kind() const = 0;

    /// The variable name
    const Identifier* const name;

    /// The declared variable type. This is null if the type is inferred, e.g.:
    ///   let f = 1.0;
    ///   var i = 1;
    const Type type;

    /// The initializer expression or nullptr if none set
    const Expression* const initializer;

    /// The attributes attached to this variable
    const utils::Vector<const Attribute*, 2> attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_VARIABLE_H_
