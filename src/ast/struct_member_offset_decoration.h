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

#include <stddef.h>

#include <string>

#include "src/ast/struct_member_decoration.h"

namespace tint {
namespace ast {

/// A struct member offset decoration
class StructMemberOffsetDecoration : public StructMemberDecoration {
 public:
  /// constructor
  /// @param offset the offset value
  explicit StructMemberOffsetDecoration(uint32_t offset);
  ~StructMemberOffsetDecoration() override;

  /// @returns true if this is an offset decoration
  bool IsOffset() const override;

  /// @returns the offset value
  uint32_t offset() const { return offset_; }

  /// @returns the decoration as a string
  std::string to_str() const override;

 private:
  uint32_t offset_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_MEMBER_OFFSET_DECORATION_H_
