// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_SEM_MODULE_H_
#define SRC_TINT_SEM_MODULE_H_

#include "src/tint/ast/extension.h"
#include "src/tint/sem/node.h"
#include "src/tint/utils/vector.h"

// Forward declarations
namespace tint::ast {
class Node;
}  // namespace tint::ast

namespace tint::sem {

/// Module holds the top-level semantic types, functions and global variables
/// used by a Program.
class Module final : public Castable<Module, Node> {
  public:
    /// Constructor
    /// @param dep_ordered_decls the dependency-ordered module-scope declarations
    /// @param extensions the list of enabled extensions in the module
    Module(utils::VectorRef<const ast::Node*> dep_ordered_decls, ast::Extensions extensions);

    /// Destructor
    ~Module() override;

    /// @returns the dependency-ordered global declarations for the module
    const utils::Vector<const ast::Node*, 64>& DependencyOrderedDeclarations() const {
        return dep_ordered_decls_;
    }

    /// @returns the list of enabled extensions in the module
    const ast::Extensions& Extensions() const { return extensions_; }

  private:
    const utils::Vector<const ast::Node*, 64> dep_ordered_decls_;
    ast::Extensions extensions_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_MODULE_H_
