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

#include <string>

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
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node
  /// @param offset the offset value
  StructMemberOffsetDecoration(ProgramID pid,
                               const Source& src,
                               uint32_t offset);
  ~StructMemberOffsetDecoration() override;

  /// @returns the WGSL name for the decoration
  std::string Name() const override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  const StructMemberOffsetDecoration* Clone(CloneContext* ctx) const override;

  /// The offset value
  const uint32_t offset;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_MEMBER_OFFSET_DECORATION_H_
