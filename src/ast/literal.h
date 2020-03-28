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

#ifndef SRC_AST_LITERAL_H_
#define SRC_AST_LITERAL_H_

#include <string>

#include "src/ast/type/type.h"

namespace tint {
namespace ast {

class BoolLiteral;
class FloatLiteral;
class IntLiteral;
class UintLiteral;

/// Base class for a literal value
class Literal {
 public:
  virtual ~Literal();

  /// @returns true if this is a bool literal
  virtual bool IsBool() const { return false; }
  /// @returns true if this is a float literal
  virtual bool IsFloat() const { return false; }
  /// @returns true if this is a signed int literal
  virtual bool IsInt() const { return false; }
  /// @returns true if this is a unsigned int literal
  virtual bool IsUint() const { return false; }

  /// @returns the literal as a boolean literal
  BoolLiteral* AsBool();
  /// @returns the literal as a float literal
  FloatLiteral* AsFloat();
  /// @returns the literal as a int literal
  IntLiteral* AsInt();
  /// @returns the literal as a unsigned int literal
  UintLiteral* AsUint();

  /// @returns the type of the literal
  ast::type::Type* type() const { return type_; }

  /// @returns the literal as a string
  virtual std::string to_str() const = 0;

  /// @returns the name for this literal. This name is unique to this value.
  virtual std::string name() const = 0;

 protected:
  /// Constructor
  explicit Literal(ast::type::Type* type);

 private:
  ast::type::Type* type_ = nullptr;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_LITERAL_H_
