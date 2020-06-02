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

#ifndef SRC_AST_SINT_LITERAL_H_
#define SRC_AST_SINT_LITERAL_H_

#include <string>

#include "src/ast/int_literal.h"

namespace tint {
namespace ast {

/// A signed int literal
class SintLiteral : public IntLiteral {
 public:
  /// Constructor
  /// @param type the type
  /// @param value the signed int literals value
  SintLiteral(ast::type::Type* type, int32_t value);
  ~SintLiteral() override;

  /// @returns true if this is a signed int literal
  bool IsSint() const override;

  /// @returns the int literal value
  int32_t value() const { return value_; }

  /// @returns the name for this literal. This name is unique to this value.
  std::string name() const override;

  /// @returns the literal as a string
  std::string to_str() const override;

 private:
  int32_t value_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_SINT_LITERAL_H_
