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

#ifndef SRC_TINT_AST_CONST_H_
#define SRC_TINT_AST_CONST_H_

#include "src/tint/ast/variable.h"

namespace tint::ast {

/// A "const" declaration is a name for a module-scoped or function-scoped creation-time value.
/// const must have a initializer expression.
///
/// Examples:
///
/// ```
///   const n  = 123;                           // Abstract-integer typed constant
///   const pi = 3.14159265359;                 // Abstract-float typed constant
///   const max_f32 : f32 = 0x1.fffffep+127;    // f32 typed constant
/// ```
/// @see https://www.w3.org/TR/WGSL/#creation-time-consts
class Const final : public utils::Castable<Const, Variable> {
  public:
    /// Create a 'const' creation-time value variable.
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the variable source
    /// @param name the variable name
    /// @param type the declared variable type
    /// @param initializer the initializer expression. Must not be nullptr.
    /// @param attributes the variable attributes
    Const(ProgramID pid,
          NodeID nid,
          const Source& source,
          const Identifier* name,
          Type type,
          const Expression* initializer,
          utils::VectorRef<const Attribute*> attributes);

    /// Destructor
    ~Const() override;

    /// @returns "const"
    const char* Kind() const override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Const* Clone(CloneContext* ctx) const override;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_CONST_H_
