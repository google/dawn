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
  void AddGlobalVariable(Variable* var) { global_variables_.push_back(var); }
  /// @returns the global variables for the module
  const VariableList& global_variables() const { return global_variables_; }

  /// @returns the global variables for the module
  VariableList& global_variables() { return global_variables_; }

  /// Adds a constructed type to the module.
  /// The type must be an alias or a struct.
  /// @param type the constructed type to add
  void AddConstructedType(type::Type* type) {
    constructed_types_.push_back(type);
  }
  /// @returns the constructed types in the module
  const std::vector<type::Type*>& constructed_types() const {
    return constructed_types_;
  }

  /// Adds a function to the module
  /// @param func the function
  void AddFunction(Function* func) { functions_.push_back(func); }
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

  /// Creates a new `ast::Node` owned by the Module. When the Module is
  /// destructed, the `ast::Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    static_assert(std::is_base_of<ast::Node, T>::value,
                  "T does not derive from ast::Node");
    auto uptr = std::make_unique<T>(std::forward<ARGS>(args)...);
    auto ptr = uptr.get();
    ast_nodes_.emplace_back(std::move(uptr));
    return ptr;
  }

 private:
  Module(const Module&) = delete;

  VariableList global_variables_;
  // The constructed types are owned by the type manager
  std::vector<type::Type*> constructed_types_;
  FunctionList functions_;
  std::vector<std::unique_ptr<ast::Node>> ast_nodes_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MODULE_H_
