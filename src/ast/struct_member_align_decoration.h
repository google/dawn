// Copyright 2021 The Tint Authors.
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

#ifndef SRC_AST_STRUCT_MEMBER_ALIGN_DECORATION_H_
#define SRC_AST_STRUCT_MEMBER_ALIGN_DECORATION_H_

#include <stddef.h>
#include <string>

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

/// A struct member align decoration
class StructMemberAlignDecoration
    : public Castable<StructMemberAlignDecoration, Decoration> {
 public:
  /// constructor
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node
  /// @param align the align value
  StructMemberAlignDecoration(ProgramID pid, const Source& src, uint32_t align);
  ~StructMemberAlignDecoration() override;

  /// @returns the WGSL name for the decoration
  std::string Name() const override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  const StructMemberAlignDecoration* Clone(CloneContext* ctx) const override;

  /// The align value
  const uint32_t align;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_MEMBER_ALIGN_DECORATION_H_
