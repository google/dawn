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

#ifndef SRC_AST_FUNCTION_H_
#define SRC_AST_FUNCTION_H_

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "src/ast/binding_decoration.h"
#include "src/ast/block_statement.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/expression.h"
#include "src/ast/location_decoration.h"
#include "src/ast/node.h"
#include "src/ast/set_decoration.h"
#include "src/ast/statement.h"
#include "src/ast/type/type.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {

/// A Function statement.
class Function : public Node {
 public:
  /// Information about a binding
  struct BindingInfo {
    /// The binding decoration
    BindingDecoration* binding = nullptr;
    /// The set decoration
    SetDecoration* set = nullptr;
  };

  /// Create a new empty function statement
  Function();
  /// Create a function
  /// @param name the function name
  /// @param params the function parameters
  /// @param return_type the return type
  Function(const std::string& name,
           VariableList params,
           type::Type* return_type);
  /// Create a function
  /// @param source the variable source
  /// @param name the function name
  /// @param params the function parameters
  /// @param return_type the return type
  Function(const Source& source,
           const std::string& name,
           VariableList params,
           type::Type* return_type);
  /// Move constructor
  Function(Function&&);

  ~Function() override;

  /// Sets the function name
  /// @param name the name to set
  void set_name(const std::string& name) { name_ = name; }
  /// @returns the function name
  const std::string& name() { return name_; }

  /// Sets the function parameters
  /// @param params the function parameters
  void set_params(VariableList params) { params_ = std::move(params); }
  /// @returns the function params
  const VariableList& params() const { return params_; }

  /// Adds the given variable to the list of referenced module variables if it
  /// is not already included.
  /// @param var the module variable to add
  void add_referenced_module_variable(Variable* var);
  /// @returns the referenced module variables
  const std::vector<Variable*>& referenced_module_variables() const {
    return referenced_module_vars_;
  }
  /// Retrieves any referenced location variables
  /// @returns the <variable, decoration> pair.
  const std::vector<std::pair<Variable*, LocationDecoration*>>
  referenced_location_variables() const;
  /// Retrieves any referenced builtin variables
  /// @returns the <variable, decoration> pair.
  const std::vector<std::pair<Variable*, BuiltinDecoration*>>
  referenced_builtin_variables() const;
  /// Retrieves any referenced uniform variables. Note, the uniform must be
  /// decorated with both binding and set decorations.
  /// @returns the referenced uniforms
  const std::vector<std::pair<Variable*, Function::BindingInfo>>
  referenced_uniform_variables() const;
  /// Retrieves any referenced storagebuffer variables. Note, the storagebuffer
  /// must be decorated with both binding and set decorations.
  /// @returns the referenced storagebuffers
  const std::vector<std::pair<Variable*, Function::BindingInfo>>
  referenced_storagebuffer_variables() const;

  /// Adds an ancestor entry point
  /// @param ep the entry point ancestor
  void add_ancestor_entry_point(const std::string& ep);
  /// @returns the ancestor entry points
  const std::vector<std::string>& ancestor_entry_points() const {
    return ancestor_entry_points_;
  }

  /// Sets the return type of the function
  /// @param type the return type
  void set_return_type(type::Type* type) { return_type_ = type; }
  /// @returns the function return type.
  type::Type* return_type() const { return return_type_; }
  /// @returns a pointer to the last statement of the function or nullptr if
  // function is empty
  const Statement* get_last_statement() const;

  /// Sets the body of the function
  /// @param body the function body
  void set_body(std::unique_ptr<BlockStatement> body) {
    body_ = std::move(body);
  }
  /// @returns the function body
  BlockStatement* body() const { return body_.get(); }

  /// @returns true if the name and type are both present
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

  /// @returns the type name for this function
  std::string type_name() const;

 private:
  Function(const Function&) = delete;

  std::string name_;
  VariableList params_;
  type::Type* return_type_ = nullptr;
  std::unique_ptr<BlockStatement> body_;
  std::vector<Variable*> referenced_module_vars_;
  std::vector<std::string> ancestor_entry_points_;
};

/// A list of unique functions
using FunctionList = std::vector<std::unique_ptr<Function>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_FUNCTION_H_
