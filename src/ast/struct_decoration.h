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

#ifndef SRC_AST_STRUCT_DECORATION_H_
#define SRC_AST_STRUCT_DECORATION_H_

#include <memory>
#include <ostream>
#include <vector>

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

/// The struct decorations
class StructDecoration : public Decoration {
 public:
  /// The kind of decoration that this type represents
  static constexpr DecorationKind Kind = DecorationKind::kStruct;

  ~StructDecoration() override;

  /// @returns true if this is a block struct
  virtual bool IsBlock() const = 0;

  /// Outputs the decoration to the given stream
  /// @param out the stream to output too
  virtual void to_str(std::ostream& out) const = 0;

 protected:
  /// Constructor
  /// @param source the source of this decoration
  explicit StructDecoration(const Source& source);
};

/// List of struct decorations
using StructDecorationList = std::vector<std::unique_ptr<StructDecoration>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_DECORATION_H_
