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

#ifndef SRC_AST_VARIABLE_H_
#define SRC_AST_VARIABLE_H_

#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/node.h"
#include "src/ast/storage_class.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {

class DecoratedVariable;

/// A variable.
class Variable : public Node {
 public:
  /// Create a new empty variable statement
  Variable() = default;
  /// Create a variable
  /// @param name the variables name
  /// @param sc the variable storage class
  /// @param type the variables type
  Variable(const std::string& name, StorageClass sc, type::Type* type);
  /// Create a variable
  /// @param source the variable source
  /// @param name the variables name
  /// @param sc the variable storage class
  /// @param type the variables type
  Variable(const Source& source,
           const std::string& name,
           StorageClass sc,
           type::Type* type);
  /// Move constructor
  Variable(Variable&&) = default;

  ~Variable() override;

  /// Sets the variable name
  /// @param name the name to set
  void set_name(const std::string& name) { name_ = name; }
  /// @returns the variable name
  const std::string& name() { return name_; }

  /// Sets the type of the variable
  /// @param type the type
  void set_type(type::Type* type) { type_ = type; }
  /// @returns the variables type.
  type::Type* type() const { return type_; }

  /// Sets the storage class
  /// @param sc the storage class
  void set_storage_class(StorageClass sc) { storage_class_ = sc; }
  /// @returns the storage class
  StorageClass storage_class() const { return storage_class_; }

  /// Sets the constructor
  /// @param expr the constructor expression
  void set_constructor(std::unique_ptr<Expression> expr) {
    constructor_ = std::move(expr);
  }
  /// @returns the constructor expression or nullptr if none set
  Expression* constructor() const { return constructor_.get(); }
  /// @returns true if the variable has an constructor
  bool has_constructor() const { return constructor_ != nullptr; }

  /// Sets if the variable is constant
  /// @param val the value to be set
  void set_is_const(bool val) { is_const_ = val; }
  /// @returns true if this is a constant, false otherwise
  bool is_const() const { return is_const_; }

  /// @returns true if this is a decorated variable
  virtual bool IsDecorated() const { return false; }

  /// @returns the expression as a decorated variable
  DecoratedVariable* AsDecorated();

  /// @returns true if the name and path are both present
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 protected:
  /// Output information for this variable.
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void info_to_str(std::ostream& out, size_t indent) const;
  /// Output constructor for this variable.
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void constructor_to_str(std::ostream& out, size_t indent) const;

 private:
  Variable(const Variable&) = delete;

  bool is_const_ = false;
  std::string name_;
  StorageClass storage_class_ = StorageClass::kNone;
  type::Type* type_ = nullptr;
  std::unique_ptr<Expression> constructor_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_VARIABLE_H_
