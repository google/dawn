// Copyright 2020 The Tint Authors.
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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/ast/function.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {

/// Represents all the source in a given program.
class Module {
 public:
  Module();
  /// Move constructor
  Module(Module&&);
  ~Module();

  /// Add a global variable to the module
  /// @param var the variable to add
  void AddGlobalVariable(std::unique_ptr<Variable> var) {
    global_variables_.push_back(std::move(var));
  }
  /// @returns the global variables for the module
  const VariableList& global_variables() const { return global_variables_; }

  /// @returns the global variables for the module
  VariableList& global_variables() { return global_variables_; }

  /// Adds a type alias to the module
  /// @param type the alias to add
  void AddAliasType(type::AliasType* type) { alias_types_.push_back(type); }
  /// @returns the alias types in the module
  const std::vector<type::AliasType*>& alias_types() const {
    return alias_types_;
  }

  /// Adds a function to the module
  /// @param func the function
  void AddFunction(std::unique_ptr<Function> func) {
    functions_.push_back(std::move(func));
  }
  /// @returns the modules functions
  const FunctionList& functions() const { return functions_; }
  /// Returns the function with the given name
  /// @param name the name to search for
  /// @returns the associated function or nullptr if none exists
  Function* FindFunctionByName(const std::string& name) const;
  /// Returns the function with the given name
  /// @param name the name to search for
  /// @param stage the pipeline stage
  /// @returns the associated function or nullptr if none exists
  Function* FindFunctionByNameAndStage(const std::string& name,
                                       ast::PipelineStage stage) const;

  /// @returns true if all required fields in the AST are present.
  bool IsValid() const;

  /// @returns a string representation of the module
  std::string to_str() const;

 private:
  Module(const Module&) = delete;

  VariableList global_variables_;
  // The alias types are owned by the type manager
  std::vector<type::AliasType*> alias_types_;
  FunctionList functions_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MODULE_H_
