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

#ifndef SRC_TINT_AST_EXTENSION_H_
#define SRC_TINT_AST_EXTENSION_H_

#include "src/tint/ast/node.h"
#include "src/tint/builtin/extension.h"

namespace tint::ast {

/// An extension used in an "enable" directive. Example:
/// ```
///   enable f16;
/// ```
class Extension final : public utils::Castable<Extension, Node> {
  public:
    /// Create a extension
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param ext the extension
    Extension(ProgramID pid, NodeID nid, const Source& src, builtin::Extension ext);

    /// Destructor
    ~Extension() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Extension* Clone(CloneContext* ctx) const override;

    /// The extension name
    const builtin::Extension name;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_EXTENSION_H_
