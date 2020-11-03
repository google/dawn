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

#ifndef SRC_AST_ARRAY_DECORATION_H_
#define SRC_AST_ARRAY_DECORATION_H_

#include <memory>
#include <string>
#include <vector>

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

class StrideDecoration;

/// A decoration attached to an array
class ArrayDecoration : public Decoration {
 public:
  /// The kind of decoration that this type represents
  static constexpr DecorationKind Kind = DecorationKind::kArray;

  ~ArrayDecoration() override;

  /// @returns true if this is a stride decoration
  virtual bool IsStride() const;

  /// @returns the decoration as a stride decoration
  StrideDecoration* AsStride();

  /// @returns the decoration as a string
  virtual std::string to_str() const = 0;

 protected:
  /// Constructor
  ArrayDecoration();
};

/// A list of unique array decorations
using ArrayDecorationList = std::vector<std::unique_ptr<ArrayDecoration>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_ARRAY_DECORATION_H_
