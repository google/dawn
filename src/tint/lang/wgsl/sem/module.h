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

#ifndef SRC_TINT_LANG_WGSL_SEM_MODULE_H_
#define SRC_TINT_LANG_WGSL_SEM_MODULE_H_

#include "src/tint/lang/core/extension.h"
#include "src/tint/lang/wgsl/ast/diagnostic_control.h"
#include "src/tint/lang/wgsl/sem/node.h"
#include "src/tint/utils/containers/vector.h"

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
    Module(VectorRef<const ast::Node*> dep_ordered_decls, core::Extensions extensions);

    /// Destructor
    ~Module() override;

    /// @returns the dependency-ordered global declarations for the module
    VectorRef<const ast::Node*> DependencyOrderedDeclarations() const { return dep_ordered_decls_; }

    /// @returns the list of enabled extensions in the module
    const core::Extensions& Extensions() const { return extensions_; }

    /// Modifies the severity of a specific diagnostic rule for this module.
    /// @param rule the diagnostic rule
    /// @param severity the new diagnostic severity
    void SetDiagnosticSeverity(core::DiagnosticRule rule, core::DiagnosticSeverity severity) {
        diagnostic_severities_[rule] = severity;
    }

    /// @returns the diagnostic severity modifications applied to this module
    const core::DiagnosticRuleSeverities& DiagnosticSeverities() const {
        return diagnostic_severities_;
    }

  private:
    const tint::Vector<const ast::Node*, 64> dep_ordered_decls_;
    core::Extensions extensions_;
    core::DiagnosticRuleSeverities diagnostic_severities_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_MODULE_H_
