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
#include <vector>

#include "src/tint/ast/enable.h"
#include "src/tint/ast/function.h"
#include "src/tint/ast/type.h"

namespace tint::ast {

class TypeDecl;

/// Module holds the top-level AST types, functions and global variables used by
/// a Program.
class Module final : public Castable<Module, Node> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node
    Module(ProgramID pid, const Source& src);

    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node
    /// @param global_decls the list of global types, functions, and variables, in
    /// the order they were declared in the source program
    Module(ProgramID pid, const Source& src, std::vector<const Node*> global_decls);

    /// Destructor
    ~Module() override;

    /// @returns the declaration-ordered global declarations for the module
    const std::vector<const Node*>& GlobalDeclarations() const { return global_declarations_; }

    /// Add a enable directive to the Builder
    /// @param ext the enable directive to add
    void AddEnable(const Enable* ext);

    /// Add a global variable to the Builder
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

    /// Adds a global declaration to the Builder.
    /// @param decl the declaration to add
    void AddGlobalDeclaration(const tint::ast::Node* decl);

    /// @returns the global variables for the module
    const VariableList& GlobalVariables() const { return global_variables_; }

    /// @returns the global variables for the module
    VariableList& GlobalVariables() { return global_variables_; }

    /// @returns the extension set for the module
    const ExtensionSet& Extensions() const { return extensions_; }

    /// Adds a type declaration to the Builder.
    /// @param decl the type declaration to add
    void AddTypeDecl(const TypeDecl* decl);

    /// @returns the TypeDecl registered as a TypeDecl()
    /// @param name the name of the type to search for
    const TypeDecl* LookupType(Symbol name) const;

    /// @returns the declared types in the module
    const std::vector<const TypeDecl*>& TypeDecls() const { return type_decls_; }

    /// Add a function to the Builder
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

    std::vector<const Node*> global_declarations_;
    std::vector<const TypeDecl*> type_decls_;
    FunctionList functions_;
    VariableList global_variables_;
    ExtensionSet extensions_;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_MODULE_H_
