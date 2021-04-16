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

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

/// A struct member offset decoration
/// @note The WGSL spec removed the `[[offset(n)]]` decoration for `[[size(n)]]`
/// and `[[align(n)]]` in https://github.com/gpuweb/gpuweb/pull/1447. However
/// this decoration is kept because the SPIR-V reader has to deal with absolute
/// offsets, and transforming these to size / align is complex and can be done
/// in a number of ways. The Resolver is responsible for consuming the size and
/// align decorations and transforming these into absolute offsets. It is
/// trivial for the Resolver to handle `[[offset(n)]]` or `[[size(n)]]` /
/// `[[align(n)]]` decorations, so this is what we do, keeping all the layout
/// logic in one place.
class StructMemberOffsetDecoration
    : public Castable<StructMemberOffsetDecoration, Decoration> {
 public:
  /// constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this decoration
  /// @param offset the offset value
  StructMemberOffsetDecoration(ProgramID program_id,
                               const Source& source,
                               uint32_t offset);
  ~StructMemberOffsetDecoration() override;

  /// @returns the offset value
  uint32_t offset() const { return offset_; }

  /// Outputs the decoration to the given stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  StructMemberOffsetDecoration* Clone(CloneContext* ctx) const override;

 private:
  uint32_t const offset_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_MEMBER_OFFSET_DECORATION_H_
