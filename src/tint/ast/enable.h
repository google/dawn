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

#ifndef SRC_TINT_AST_ENABLE_H_
#define SRC_TINT_AST_ENABLE_H_

#include <string>
#include <utility>
#include <vector>

#include "src/tint/ast/extension.h"

namespace tint::ast {

/// An "enable" directive. Example:
/// ```
///   // Enable an extension named "f16"
///   enable f16;
/// ```
class Enable final : public utils::Castable<Enable, Node> {
  public:
    /// Create a extension
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param exts the extensions being enabled by this directive
    Enable(ProgramID pid, NodeID nid, const Source& src, utils::VectorRef<const Extension*> exts);

    /// Destructor
    ~Enable() override;

    /// @param ext the extension to search for
    /// @returns true if this Enable lists the given extension
    bool HasExtension(builtin::Extension ext) const;

    /// Clones this node and all transitive child nodes using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Enable* Clone(CloneContext* ctx) const override;

    /// The extensions being enabled by this directive
    const utils::Vector<const Extension*, 4> extensions;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_ENABLE_H_
