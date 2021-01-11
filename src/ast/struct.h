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

#ifndef SRC_AST_STRUCT_H_
#define SRC_AST_STRUCT_H_

#include <ostream>
#include <string>
#include <utility>

#include "src/ast/node.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"

namespace tint {
namespace ast {

/// A struct statement.
class Struct : public Castable<Struct, Node> {
 public:
  /// Create a new struct statement
  /// @param source The input source for the import statement
  /// @param members The struct members
  /// @param decorations The struct decorations
  Struct(const Source& source,
         StructMemberList members,
         StructDecorationList decorations);
  /// Move constructor
  Struct(Struct&&);

  ~Struct() override;

  /// @returns the struct decorations
  const StructDecorationList& decorations() const { return decorations_; }

  /// @returns the members
  const StructMemberList& members() const { return members_; }

  /// Returns the struct member with the given symbol or nullptr if non exists.
  /// @param symbol the symbol of the member
  /// @returns the struct member or nullptr if not found
  StructMember* get_member(const Symbol& symbol) const;

  /// @returns true if the struct is block decorated
  bool IsBlockDecorated() const;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param ctx the clone context
  /// @return the newly cloned node
  Struct* Clone(CloneContext* ctx) const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  Struct(const Struct&) = delete;

  StructMemberList const members_;
  StructDecorationList const decorations_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_H_
