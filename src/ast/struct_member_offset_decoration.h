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

#ifndef SRC_AST_STRUCT_MEMBER_OFFSET_DECORATION_H_
#define SRC_AST_STRUCT_MEMBER_OFFSET_DECORATION_H_

#include <stddef.h>

#include <string>

#include "src/ast/struct_member_decoration.h"

namespace tint {
namespace ast {

/// A struct member offset decoration
class StructMemberOffsetDecoration : public StructMemberDecoration {
 public:
  /// The kind of decoration that this type represents
  static constexpr const DecorationKind Kind =
      DecorationKind::kStructMemberOffset;

  /// constructor
  /// @param offset the offset value
  /// @param source the source of this decoration
  StructMemberOffsetDecoration(uint32_t offset, const Source& source);
  ~StructMemberOffsetDecoration() override;

  /// @return the decoration kind
  DecorationKind GetKind() const override;

  /// @param kind the decoration kind
  /// @return true if this Decoration is of the (or derives from) the given
  /// kind.
  bool IsKind(DecorationKind kind) const override;

  /// @returns true if this is an offset decoration
  bool IsOffset() const override;

  /// @returns the offset value
  uint32_t offset() const { return offset_; }

  /// Outputs the decoration to the given stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  uint32_t offset_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_MEMBER_OFFSET_DECORATION_H_
