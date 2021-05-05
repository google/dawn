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

#ifndef SRC_AST_MODULE_H_
#define SRC_AST_MODULE_H_

#include <string>
#include <vector>

#include "src/ast/function.h"
#include "src/ast/type.h"

namespace tint {
namespace ast {

class NamedType;

/// Module holds the top-level AST types, functions and global variables used by
/// a Program.
class Module : public Castable<Module, Node> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of the module
  Module(ProgramID program_id, const Source& source);

  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of the module
  /// @param global_decls the list of global types, functions, and variables, in
  /// the order they were declared in the source program
  Module(ProgramID program_id,
         const Source& source,
         std::vector<ast::Node*> global_decls);

  /// Destructor
  ~Module() override;

  /// @returns the ordered global declarations for the translation unit
  const std::vector<ast::Node*>& GlobalDeclarations() const {
    return global_declarations_;
  }

  /// Add a global variable to the Builder
  /// @param var the variable to add
  void AddGlobalVariable(ast::Variable* var);

  /// @returns true if the module has the global declaration `decl`
  /// @param decl the declaration to check
  bool HasGlobalDeclaration(ast::Node* decl) const {
    for (auto* d : global_declarations_) {
      if (d == decl) {
        return true;
      }
    }
    return false;
  }

  /// @returns the global variables for the translation unit
  const VariableList& GlobalVariables() const { return global_variables_; }

  /// @returns the global variables for the translation unit
  VariableList& GlobalVariables() { return global_variables_; }

  /// Adds a constructed type to the Builder.
  /// @param type the constructed type to add
  void AddConstructedType(ast::NamedType* type);

  /// @returns the NamedType registered as a ConstructedType()
  /// @param name the name of the type to search for
  const ast::NamedType* LookupType(Symbol name) const;

  /// @returns the constructed types in the translation unit
  const std::vector<ast::NamedType*>& ConstructedTypes() const {
    return constructed_types_;
  }

  /// Add a function to the Builder
  /// @param func the function to add
  void AddFunction(ast::Function* func);

  /// @returns the functions declared in the translation unit
  const FunctionList& Functions() const { return functions_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  Module* Clone(CloneContext* ctx) const override;

  /// Copy copies the content of the Module src into this module.
  /// @param ctx the clone context
  /// @param src the module to copy into this module
  void Copy(CloneContext* ctx, const Module* src);

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

  /// @param sem the semantic info for the program
  /// @returns a string representation of the Builder
  std::string to_str(const sem::Info& sem) const;

 private:
  std::vector<ast::Node*> global_declarations_;
  std::vector<ast::NamedType*> constructed_types_;
  FunctionList functions_;
  VariableList global_variables_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MODULE_H_
