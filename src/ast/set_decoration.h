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

#ifndef SRC_AST_SET_DECORATION_H_
#define SRC_AST_SET_DECORATION_H_

#include <stddef.h>

#include "src/ast/variable_decoration.h"

namespace tint {
namespace ast {

/// A set decoration
class SetDecoration : public VariableDecoration {
 public:
  /// constructor
  /// @param value the set value
  explicit SetDecoration(uint32_t value);
  ~SetDecoration() override;

  /// @returns true if this is a set decoration
  bool IsSet() const override;

  /// @returns the set value
  uint32_t value() const { return value_; }

  /// Outputs the decoration to the given stream
  /// @param out the stream to output too
  void to_str(std::ostream& out) const override;

 private:
  uint32_t value_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_SET_DECORATION_H_
