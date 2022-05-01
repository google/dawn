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

#ifndef SRC_TINT_AST_TYPE_NAME_H_
#define SRC_TINT_AST_TYPE_NAME_H_

#include <string>

#include "src/tint/ast/type.h"

namespace tint::ast {

/// A named type (i.e. struct or alias)
class TypeName final : public Castable<TypeName, Type> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node
    /// @param name the type name
    TypeName(ProgramID pid, const Source& src, Symbol name);
    /// Move constructor
    TypeName(TypeName&&);
    /// Destructor
    ~TypeName() override;

    /// @param symbols the program's symbol table
    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const SymbolTable& symbols) const override;

    /// Clones this type and all transitive types using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned type
    const TypeName* Clone(CloneContext* ctx) const override;

    /// The type name
    Symbol name;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_TYPE_NAME_H_
