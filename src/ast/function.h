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

#include "src/ast/expression.h"
#include "src/ast/node.h"
#include "src/ast/statement.h"
#include "src/ast/type/type.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {

/// A Function statement.
class Function : public Node {
 public:
  /// Create a new empty function statement
  Function() = default;
  /// Create a function
  /// @param name the function name
  /// @param params the function parameters
  /// @param return_type the return type
  Function(const std::string& name,
           std::vector<std::unique_ptr<Variable>> params,
           type::Type* return_type);
  /// Create a function
  /// @param source the variable source
  /// @param name the function name
  /// @param params the function parameters
  /// @param return_type the return type
  Function(const Source& source,
           const std::string& name,
           std::vector<std::unique_ptr<Variable>> params,
           type::Type* return_type);
  /// Move constructor
  Function(Function&&) = default;

  ~Function() override;

  /// Sets the function name
  /// @param name the name to set
  void set_name(const std::string& name) { name_ = name; }
  /// @returns the function name
  const std::string& name() { return name_; }

  /// Sets the function parameters
  /// @param params the function parameters
  void set_params(std::vector<std::unique_ptr<Variable>> params) {
    params_ = std::move(params);
  }
  /// @returns the function params
  const std::vector<std::unique_ptr<Variable>>& params() const {
    return params_;
  }

  /// Sets the return type of the function
  /// @param type the return type
  void set_return_type(type::Type* type) { return_type_ = type; }
  /// @returns the function return type.
  type::Type* return_type() const { return return_type_; }

  /// Sets the body of the function
  /// @param body the function body
  void set_body(std::vector<std::unique_ptr<Statement>> body) {
    body_ = std::move(body);
  }
  /// @returns the function body
  const std::vector<std::unique_ptr<Statement>>& body() const { return body_; }

  /// @returns true if the name and type are both present
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  Function(const Function&) = delete;

  std::string name_;
  std::vector<std::unique_ptr<Variable>> params_;
  type::Type* return_type_ = nullptr;
  std::vector<std::unique_ptr<Statement>> body_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_FUNCTION_H_
