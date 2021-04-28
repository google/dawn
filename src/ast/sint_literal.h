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
class SintLiteral : public Castable<SintLiteral, IntLiteral> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the input source
  /// @param value the signed int literals value
  SintLiteral(ProgramID program_id, const Source& source, int32_t value);
  ~SintLiteral() override;

  /// @returns the int literal value
  int32_t value() const { return static_cast<int32_t>(value_as_u32()); }

  /// @returns the name for this literal. This name is unique to this value.
  std::string name() const override;

  /// @param sem the semantic info for the program
  /// @returns the literal as a string
  std::string to_str(const sem::Info& sem) const override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  SintLiteral* Clone(CloneContext* ctx) const override;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_SINT_LITERAL_H_
