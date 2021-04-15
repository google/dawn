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

namespace tint {
namespace ast {

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
         std::vector<Cloneable*> global_decls);

  /// Destructor
  ~Module() override;

  /// @returns the ordered global declarations for the translation unit
  const std::vector<Cloneable*>& GlobalDeclarations() const {
    return global_declarations_;
  }

  /// Add a global variable to the Builder
  /// @param var the variable to add
  void AddGlobalVariable(ast::Variable* var) {
    TINT_ASSERT(var);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(var, program_id());
    global_variables_.push_back(var);
    global_declarations_.push_back(var);
  }

  /// @returns the global variables for the translation unit
  const VariableList& GlobalVariables() const { return global_variables_; }

  /// @returns the global variables for the translation unit
  VariableList& GlobalVariables() { return global_variables_; }

  /// Adds a constructed type to the Builder.
  /// The type must be an alias or a struct.
  /// @param type the constructed type to add
  void AddConstructedType(type::Type* type) {
    TINT_ASSERT(type);
    constructed_types_.push_back(type);
    global_declarations_.push_back(type);
  }

  /// @returns the constructed types in the translation unit
  const std::vector<type::Type*>& ConstructedTypes() const {
    return constructed_types_;
  }

  /// Add a function to the Builder
  /// @param func the function to add
  void AddFunction(ast::Function* func) {
    TINT_ASSERT(func);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(func, program_id());
    functions_.push_back(func);
    global_declarations_.push_back(func);
  }

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
  void to_str(const semantic::Info& sem,
              std::ostream& out,
              size_t indent) const override;

  /// @param sem the semantic info for the program
  /// @returns a string representation of the Builder
  std::string to_str(const semantic::Info& sem) const;

 private:
  std::vector<Cloneable*> global_declarations_;
  std::vector<type::Type*> constructed_types_;
  FunctionList functions_;
  VariableList global_variables_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MODULE_H_
