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

#ifndef SRC_AST_INT_LITERAL_H_
#define SRC_AST_INT_LITERAL_H_

#include "src/ast/literal.h"

namespace tint {
namespace ast {

/// An integer literal. This could be either signed or unsigned.
class IntLiteral : public Castable<IntLiteral, Literal> {
 public:
  ~IntLiteral() override;

  /// @returns the literal value as an i32
  int32_t value_as_i32() const { return static_cast<int32_t>(value_); }

  /// @returns the literal value as a u32
  uint32_t value_as_u32() const { return value_; }

 protected:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the input source
  /// @param value value of the literal
  IntLiteral(ProgramID program_id, const Source& source, uint32_t value);

 private:
  uint32_t const value_;
};  // namespace ast

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_INT_LITERAL_H_
