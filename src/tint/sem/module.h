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

#include <vector>

#include "src/tint/sem/node.h"

// Forward declarations
namespace tint::ast {
class Node;
class Module;
}  // namespace tint::ast

namespace tint::sem {

/// Module holds the top-level semantic types, functions and global variables
/// used by a Program.
class Module final : public Castable<Module, Node> {
  public:
    /// Constructor
    /// @param dep_ordered_decls the dependency-ordered module-scope declarations
    explicit Module(std::vector<const ast::Node*> dep_ordered_decls);

    /// Destructor
    ~Module() override;

    /// @returns the dependency-ordered global declarations for the module
    const std::vector<const ast::Node*>& DependencyOrderedDeclarations() const {
        return dep_ordered_decls_;
    }

  private:
    const std::vector<const ast::Node*> dep_ordered_decls_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_MODULE_H_
