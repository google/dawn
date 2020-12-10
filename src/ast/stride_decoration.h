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

#ifndef SRC_AST_STRIDE_DECORATION_H_
#define SRC_AST_STRIDE_DECORATION_H_

#include <stddef.h>

#include "src/ast/array_decoration.h"

namespace tint {
namespace ast {

/// A stride decoration
class StrideDecoration : public Castable<StrideDecoration, ArrayDecoration> {
 public:
  /// constructor
  /// @param stride the stride value
  /// @param source the source of this decoration
  StrideDecoration(uint32_t stride, const Source& source);
  ~StrideDecoration() override;

  /// @returns the stride value
  uint32_t stride() const { return stride_; }

  /// Outputs the decoration to the given stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param ctx the clone context
  /// @return the newly cloned node
  StrideDecoration* Clone(CloneContext* ctx) const override;

 private:
  uint32_t stride_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRIDE_DECORATION_H_
