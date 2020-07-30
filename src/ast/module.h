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

#include "src/ast/entry_point.h"
#include "src/ast/function.h"
#include "src/ast/import.h"
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

  /// Add the given import to the module
  /// @param import The import to add.
  void AddImport(std::unique_ptr<Import> import) {
    imports_.push_back(std::move(import));
  }
  /// @returns the imports for this module
  const ImportList& imports() const { return imports_; }
  /// Find the import of the given name
  /// @param name The import name to search for
  /// @returns the import with the given name if found, nullptr otherwise.
  Import* FindImportByName(const std::string& name) const;

  /// Add a global variable to the module
  /// @param var the variable to add
  void AddGlobalVariable(std::unique_ptr<Variable> var) {
    global_variables_.push_back(std::move(var));
  }
  /// @returns the global variables for the module
  const VariableList& global_variables() const { return global_variables_; }

  /// @returns the global variables for the module
  VariableList& global_variables() { return global_variables_; }

  /// Adds an entry point to the module
  /// @param ep the entry point to add
  void AddEntryPoint(std::unique_ptr<EntryPoint> ep) {
    entry_points_.push_back(std::move(ep));
  }
  /// @returns the entry points in the module
  const EntryPointList& entry_points() const { return entry_points_; }

  /// Checks if the given function name is an entry point function
  /// @param name the function name
  /// @returns true if name is an entry point function
  bool IsFunctionEntryPoint(const std::string& name) const;

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

  /// @returns true if all required fields in the AST are present.
  bool IsValid() const;

  /// @returns a string representation of the module
  std::string to_str() const;

 private:
  Module(const Module&) = delete;

  ImportList imports_;
  VariableList global_variables_;
  EntryPointList entry_points_;
  // The alias types are owned by the type manager
  std::vector<type::AliasType*> alias_types_;
  FunctionList functions_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MODULE_H_
