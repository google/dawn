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

#ifndef SRC_AST_BUILTIN_DECORATION_H_
#define SRC_AST_BUILTIN_DECORATION_H_

#include "src/ast/builtin.h"
#include "src/ast/variable_decoration.h"

namespace tint {
namespace ast {

/// A builtin decoration
class BuiltinDecoration
    : public Castable<BuiltinDecoration, VariableDecoration> {
 public:
  /// constructor
  /// @param builtin the builtin value
  /// @param source the source of this decoration
  BuiltinDecoration(Builtin builtin, const Source& source);
  ~BuiltinDecoration() override;

  /// @returns the builtin value
  Builtin value() const { return builtin_; }

  /// Outputs the decoration to the given stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  Builtin builtin_ = Builtin::kNone;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_BUILTIN_DECORATION_H_
