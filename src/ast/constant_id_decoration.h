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

#ifndef SRC_AST_CONSTANT_ID_DECORATION_H_
#define SRC_AST_CONSTANT_ID_DECORATION_H_

#include "src/ast/builtin.h"
#include "src/ast/variable_decoration.h"

namespace tint {
namespace ast {

/// A constant id decoration
class ConstantIdDecoration
    : public Castable<ConstantIdDecoration, VariableDecoration> {
 public:
  /// The kind of decoration that this type represents
  static constexpr const DecorationKind Kind = DecorationKind::kConstantId;

  /// constructor
  /// @param val the constant_id value
  /// @param source the source of this decoration
  ConstantIdDecoration(uint32_t val, const Source& source);
  ~ConstantIdDecoration() override;

  /// @return the decoration kind
  DecorationKind GetKind() const override;

  /// @returns true if this is a constant_id decoration
  bool IsConstantId() const override;

  /// @returns the constant id value
  uint32_t value() const { return value_; }

  /// Outputs the decoration to the given stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  uint32_t value_ = 0;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CONSTANT_ID_DECORATION_H_
