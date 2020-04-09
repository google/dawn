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

#ifndef SRC_AST_TYPE_F32_TYPE_H_
#define SRC_AST_TYPE_F32_TYPE_H_

#include <string>

#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// A float 32 type
class F32Type : public Type {
 public:
  /// Constructor
  F32Type();
  /// Move constructor
  F32Type(F32Type&&) = default;
  ~F32Type() override;

  /// @returns true if the type is an f32 type
  bool IsF32() const override;

  /// @returns the name for this type
  std::string type_name() const override;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_F32_TYPE_H_
