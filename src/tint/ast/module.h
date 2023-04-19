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

#ifndef SRC_TINT_AST_MODULE_H_
#define SRC_TINT_AST_MODULE_H_

#include <string>

#include "src/tint/ast/const_assert.h"
#include "src/tint/ast/diagnostic_directive.h"
#include "src/tint/ast/enable.h"
#include "src/tint/ast/function.h"
#include "src/tint/utils/vector.h"

namespace tint::ast {

class TypeDecl;

/// Module holds the top-level AST types, functions and global variables used by
/// a Program.
class Module final : public utils::Castable<Module, Node> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    Module(ProgramID pid, NodeID nid, const Source& src);

    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param global_decls the list of global types, functions, and variables, in
    /// the order they were declared in the source program
    Module(ProgramID pid,
           NodeID nid,
           const Source& src,
           utils::VectorRef<const Node*> global_decls);

    /// Destructor
    ~Module() override;

    /// @returns the declaration-ordered global declarations for the module
    const auto& GlobalDeclarations() const { return global_declarations_; }

    /// Add a global variable to the module
    /// @param var the variable to add
    void AddGlobalVariable(const Variable* var);

    /// @returns true if the module has the global declaration `decl`
    /// @param decl the declaration to check
    bool HasGlobalDeclaration(Node* decl) const {
        for (auto* d : global_declarations_) {
            if (d == decl) {
                return true;
            }
        }
        return false;
    }

    /// Adds a global declaration to the module.
    /// @param decl the declaration to add
    void AddGlobalDeclaration(const tint::ast::Node* decl);

    /// @returns the global variables for the module
    const auto& GlobalVariables() const { return global_variables_; }

    /// @returns the global variables for the module
    auto& GlobalVariables() { return global_variables_; }

    /// @returns the global variable declarations of kind 'T' for the module
    template <typename T, typename = utils::traits::EnableIfIsType<T, Variable>>
    auto Globals() const {
        utils::Vector<const T*, 32> out;
        out.Reserve(global_variables_.Length());
        for (auto* global : global_variables_) {
            if (auto* var = global->As<T>()) {
                out.Push(var);
            }
        }
        return out;
    }

    /// Add a diagnostic directive to the module
    /// @param diagnostic the diagnostic directive to add
    void AddDiagnosticDirective(const DiagnosticDirective* diagnostic);

    /// Add a enable directive to the module
    /// @param ext the enable directive to add
    void AddEnable(const Enable* ext);

    /// @returns the diagnostic directives for the module
    const auto& DiagnosticDirectives() const { return diagnostic_directives_; }

    /// @returns the extension set for the module
    const auto& Enables() const { return enables_; }

    /// Add a global const assertion to the module
    /// @param assertion the const assert to add
    void AddConstAssert(const ConstAssert* assertion);

    /// @returns the list of global const assertions
    const auto& ConstAsserts() const { return const_asserts_; }

    /// Adds a type declaration to the module
    /// @param decl the type declaration to add
    void AddTypeDecl(const TypeDecl* decl);

    /// @returns the TypeDecl registered as a TypeDecl()
    /// @param name the name of the type to search for
    const TypeDecl* LookupType(Symbol name) const;

    /// @returns the declared types in the module
    const auto& TypeDecls() const { return type_decls_; }

    /// Add a function to the module
    /// @param func the function to add
    void AddFunction(const Function* func);

    /// @returns the functions declared in the module
    const FunctionList& Functions() const { return functions_; }

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Module* Clone(CloneContext* ctx) const override;

    /// Copy copies the content of the Module src into this module.
    /// @param ctx the clone context
    /// @param src the module to copy into this module
    void Copy(CloneContext* ctx, const Module* src);

  private:
    /// Adds `decl` to either:
    /// * #global_declarations_
    /// * #type_decls_
    /// * #functions_
    void BinGlobalDeclaration(const tint::ast::Node* decl, diag::List& diags);

    utils::Vector<const Node*, 64> global_declarations_;
    utils::Vector<const TypeDecl*, 16> type_decls_;
    FunctionList functions_;
    utils::Vector<const Variable*, 32> global_variables_;
    utils::Vector<const DiagnosticDirective*, 8> diagnostic_directives_;
    utils::Vector<const Enable*, 8> enables_;
    utils::Vector<const ConstAssert*, 8> const_asserts_;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_MODULE_H_
