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

#ifndef SRC_AST_WORKGROUP_DECORATION_H_
#define SRC_AST_WORKGROUP_DECORATION_H_

#include <stddef.h>

#include <tuple>

#include "src/ast/function_decoration.h"

namespace tint {
namespace ast {

/// A workgroup decoration
class WorkgroupDecoration
    : public Castable<WorkgroupDecoration, FunctionDecoration> {
 public:
  /// constructor
  /// @param source the source of this decoration
  /// @param x the workgroup x dimension size
  WorkgroupDecoration(const Source& source, uint32_t x);
  /// constructor
  /// @param source the source of this decoration
  /// @param x the workgroup x dimension size
  /// @param y the workgroup x dimension size
  WorkgroupDecoration(const Source& source, uint32_t x, uint32_t y);
  /// constructor
  /// @param source the source of this decoration
  /// @param x the workgroup x dimension size
  /// @param y the workgroup x dimension size
  /// @param z the workgroup x dimension size
  WorkgroupDecoration(const Source& source, uint32_t x, uint32_t y, uint32_t z);
  ~WorkgroupDecoration() override;

  /// @returns the workgroup dimensions
  std::tuple<uint32_t, uint32_t, uint32_t> values() const {
    return {x_, y_, z_};
  }

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
  WorkgroupDecoration* Clone(CloneContext* ctx) const override;

 private:
  uint32_t x_ = 1;
  uint32_t y_ = 1;
  uint32_t z_ = 1;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_WORKGROUP_DECORATION_H_
