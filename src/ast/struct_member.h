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

#ifndef SRC_AST_STRUCT_MEMBER_H_
#define SRC_AST_STRUCT_MEMBER_H_

#include <utility>
#include <vector>

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

// Forward declaration
class Type;

/// A struct member statement.
class StructMember : public Castable<StructMember, Node> {
 public:
  /// Create a new struct member statement
  /// @param program_id the identifier of the program that owns this node
  /// @param source The input source for the struct member statement
  /// @param sym The struct member symbol
  /// @param type The struct member type
  /// @param decorations The struct member decorations
  StructMember(ProgramID program_id,
               const Source& source,
               const Symbol& sym,
               ast::Type* type,
               DecorationList decorations);
  /// Move constructor
  StructMember(StructMember&&);

  ~StructMember() override;

  /// @returns the symbol
  const Symbol& symbol() const { return symbol_; }

  /// @returns the type
  ast::Type* type() const { return type_; }

  /// @returns the decorations
  const DecorationList& decorations() const { return decorations_; }

  /// @returns true if the struct member has an offset decoration
  bool has_offset_decoration() const;
  /// @returns the offset decoration value.
  uint32_t offset() const;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  StructMember* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  StructMember(const StructMember&) = delete;

  Symbol const symbol_;
  ast::Type* const type_;
  DecorationList const decorations_;
};

/// A list of struct members
using StructMemberList = std::vector<StructMember*>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_MEMBER_H_
