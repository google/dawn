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

#ifndef SRC_TINT_LANG_WGSL_AST_ALIAS_H_
#define SRC_TINT_LANG_WGSL_AST_ALIAS_H_

#include <string>

#include "src/tint/lang/wgsl/ast/type.h"
#include "src/tint/lang/wgsl/ast/type_decl.h"

namespace tint::ast {

/// A type alias type. Holds a name and pointer to another type.
class Alias final : public Castable<Alias, TypeDecl> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param name the symbol for the alias
    /// @param subtype the alias'd type
    Alias(GenerationID pid, NodeID nid, const Source& src, const Identifier* name, Type subtype);

    /// Destructor
    ~Alias() override;

    /// Clones this type and all transitive types using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned type
    const Alias* Clone(CloneContext& ctx) const override;

    /// the alias type
    const Type type;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_ALIAS_H_
