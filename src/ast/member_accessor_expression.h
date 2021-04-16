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

#ifndef SRC_AST_MEMBER_ACCESSOR_EXPRESSION_H_
#define SRC_AST_MEMBER_ACCESSOR_EXPRESSION_H_

#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {

/// A member accessor expression
class MemberAccessorExpression
    : public Castable<MemberAccessorExpression, Expression> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the member accessor expression source
  /// @param structure the structure
  /// @param member the member
  MemberAccessorExpression(ProgramID program_id,
                           const Source& source,
                           Expression* structure,
                           IdentifierExpression* member);
  /// Move constructor
  MemberAccessorExpression(MemberAccessorExpression&&);
  ~MemberAccessorExpression() override;

  /// @returns the structure
  Expression* structure() const { return struct_; }
  /// @returns the member expression
  IdentifierExpression* member() const { return member_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  MemberAccessorExpression* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  MemberAccessorExpression(const MemberAccessorExpression&) = delete;

  Expression* const struct_;
  IdentifierExpression* const member_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MEMBER_ACCESSOR_EXPRESSION_H_
