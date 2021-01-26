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
#include "src/ast/node.h"
#include "src/ast/variable.h"
#include "src/type/type.h"

namespace tint {
namespace ast {

/// Module holds the top-level AST types, functions and global variables used by
/// a Program.
class Module : public Castable<Module, Node> {
 public:
  /// Constructor
  /// @param source the source of the module
  explicit Module(const Source& source);

  /// Constructor
  /// @param source the source of the module
  /// @param constructed_types the list of types explicitly declared in the AST
  /// @param functions the list of program functions
  /// @param global_variables the list of global variables
  Module(const Source& source,
         std::vector<type::Type*> constructed_types,
         FunctionList functions,
         VariableList global_variables);

  /// Destructor
  ~Module() override;

  /// Add a global variable to the Builder
  /// @param var the variable to add
  void AddGlobalVariable(ast::Variable* var) {
    global_variables_.push_back(var);
  }

  /// @returns the global variables for the translation unit
  const VariableList& GlobalVariables() const { return global_variables_; }

  /// @returns the global variables for the translation unit
  VariableList& GlobalVariables() { return global_variables_; }

  /// Adds a constructed type to the Builder.
  /// The type must be an alias or a struct.
  /// @param type the constructed type to add
  void AddConstructedType(type::Type* type) {
    constructed_types_.push_back(type);
  }

  /// @returns the constructed types in the translation unit
  const std::vector<type::Type*>& ConstructedTypes() const {
    return constructed_types_;
  }

  /// @returns the functions declared in the translation unit
  const FunctionList& Functions() const { return functions_; }

  /// @returns the functions declared in the translation unit
  FunctionList& Functions() { return functions_; }

  /// @returns true if all required fields in the AST are present.
  bool IsValid() const override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param ctx the clone context
  /// @return the newly cloned node
  Module* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

  /// @returns a string representation of the Builder
  std::string to_str() const;

 private:
  std::vector<type::Type*> constructed_types_;
  FunctionList functions_;
  VariableList global_variables_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MODULE_H_
