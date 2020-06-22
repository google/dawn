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
#include <vector>

#include "src/ast/expression.h"
#include "src/ast/node.h"
#include "src/ast/storage_class.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {

class DecoratedVariable;

/// A Variable statement.
///
/// An instance of this class represents one of three constructs in WGSL: "var"
/// declaration, "const" declaration, or formal parameter to a function.
///
/// 1. A "var" declaration is a name for typed storage.  Examples:
///
///       // Declared outside a function, i.e. at module scope, requires
///       // a storage class.
///       var<workgroup> width : i32;     // no initializer
///       var<private> height : i32 = 3;  // with initializer
///
///       // A variable declared inside a function doesn't take a storage class,
///       // and maps to SPIR-V Function storage.
///       var computed_depth : i32;
///       var area : i32 = compute_area(width, height);
///
/// 2. A "const" declaration is a name for a typed value.  Examples:
///
///       const twice_depth : i32 = width + width;  // Must have initializer
///
/// 3. A formal parameter to a function is a name for a typed value to
///    be passed into a function.  Example:
///
///       fn twice(a: i32) -> i32 {  // "a:i32" is the formal parameter
///         return a + a;
///       }
///
/// From the WGSL draft, about "var"::
///
///   A variable is a named reference to storage that can contain a value of a
///   particular type.
///
///   Two types are associated with a variable: its store type (the type of
///   value that may be placed in the referenced storage) and its reference
///   type (the type of the variable itself).  If a variable has store type T
///   and storage class S, then its reference type is pointer-to-T-in-S.
///
/// This class uses the term "type" to refer to:
///     the value type of a "const",
///     the value type of the formal parameter,
///     or the store type of the "var".
///
/// The storage class for a "var" is StorageClass::kNone when using the
/// defaulting syntax for a "var" declared inside a function.
/// The storage class for a "const" is always StorageClass::kNone.
/// The storage class for a formal parameter is always StorageClass::kNone.
class Variable : public Node {
 public:
  /// Create a new empty variable statement
  Variable();
  /// Create a variable
  /// @param name the variables name
  /// @param sc the variable storage class
  /// @param type the value type
  Variable(const std::string& name, StorageClass sc, type::Type* type);
  /// Create a variable
  /// @param source the variable source
  /// @param name the variables name
  /// @param sc the variable storage class
  /// @param type the value type
  Variable(const Source& source,
           const std::string& name,
           StorageClass sc,
           type::Type* type);
  /// Move constructor
  Variable(Variable&&);

  ~Variable() override;

  /// Sets the variable name
  /// @param name the name to set
  void set_name(const std::string& name) { name_ = name; }
  /// @returns the variable name
  const std::string& name() const { return name_; }

  /// Sets the value type if a const or formal parameter, or the
  /// store type if a var.
  /// @param type the type
  void set_type(type::Type* type) { type_ = type; }
  /// @returns the variable's type.
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
  virtual bool IsDecorated() const;

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
  // The value type if a const or formal paramter, and the store type if a var
  type::Type* type_ = nullptr;
  std::unique_ptr<Expression> constructor_;
};

/// A list of unique variables
using VariableList = std::vector<std::unique_ptr<Variable>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_VARIABLE_H_
