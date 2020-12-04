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
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/function.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type_manager.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {

/// Represents all the source in a given program.
class Module {
  template <typename T, typename BASE>
  using EnableIfIsType =
      typename std::enable_if<std::is_base_of<BASE, T>::value, T>::type;

 public:
  /// Constructor
  Module();

  /// Move constructor
  Module(Module&&);

  /// Move assignment operator
  /// @param rhs the Module to move
  /// @return this Module
  Module& operator=(Module&& rhs);

  /// Destructor
  ~Module();

  /// @return a deep copy of this module
  Module Clone();

  /// Clone this module into `ctx->mod` using the provided CloneContext
  /// @param ctx the clone context
  void Clone(CloneContext* ctx);

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
                                       PipelineStage stage) const;
  /// @param stage the pipeline stage
  /// @returns true if the module contains an entrypoint function with the given
  /// stage
  bool HasStage(PipelineStage stage) const;

  /// @returns true if all required fields in the AST are present.
  bool IsValid() const;

  /// @returns a string representation of the module
  std::string to_str() const;

  /// Creates a new `Node` owned by the Module. When the Module is
  /// destructed, the `Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  EnableIfIsType<T, Node>* create(ARGS&&... args) {
    static_assert(std::is_base_of<Node, T>::value,
                  "T does not derive from Node");
    auto uptr = std::make_unique<T>(std::forward<ARGS>(args)...);
    auto ptr = uptr.get();
    ast_nodes_.emplace_back(std::move(uptr));
    return ptr;
  }

  /// Creates a new `Type` owned by the Module.
  /// When the Module is destructed, owned Module and the returned
  /// `Type` will also be destructed.
  /// Types are unique (de-aliased), and so calling create() for the same `T`
  /// and arguments will return the same pointer.
  /// @warning Use this method to acquire a type only if all of its type
  /// information is provided in the constructor arguments `args`.<br>
  /// If the type requires additional configuration after construction that
  /// affect its fundamental type, build the type with `std::make_unique`, make
  /// any necessary alterations and then call unique_type() instead.
  /// @param args the arguments to pass to the type constructor
  /// @returns the de-aliased type pointer
  template <typename T, typename... ARGS>
  EnableIfIsType<T, type::Type>* create(ARGS&&... args) {
    static_assert(std::is_base_of<type::Type, T>::value,
                  "T does not derive from type::Type");
    return type_mgr_.Get<T>(std::forward<ARGS>(args)...);
  }

  /// Moves the type `ty` to the Module, returning a pointer to the unique
  /// (de-aliased) type.
  /// When the Module is destructed, the returned `Type` will also be
  /// destructed.
  /// @see create()
  /// @param ty the type to add to the module
  /// @returns the de-aliased type pointer
  template <typename T>
  EnableIfIsType<T, type::Type>* unique_type(std::unique_ptr<T> ty) {
    return static_cast<T*>(type_mgr_.Get(std::move(ty)));
  }

  /// Returns all the declared types in the module
  /// @returns the mapping from name string to type.
  const std::unordered_map<std::string, std::unique_ptr<type::Type>>& types() {
    return type_mgr_.types();
  }

  /// @returns all the declared nodes in the module
  const std::vector<std::unique_ptr<ast::Node>>& nodes() { return ast_nodes_; }

 private:
  Module(const Module&) = delete;

  VariableList global_variables_;
  // The constructed types are owned by the type manager
  std::vector<type::Type*> constructed_types_;
  FunctionList functions_;
  std::vector<std::unique_ptr<Node>> ast_nodes_;
  TypeManager type_mgr_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MODULE_H_
