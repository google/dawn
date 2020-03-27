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

#ifndef SRC_AST_TYPE_ARRAY_TYPE_H_
#define SRC_AST_TYPE_ARRAY_TYPE_H_

#include <assert.h>

#include <string>

#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// An array type. If size is zero then it is a runtime array.
class ArrayType : public Type {
 public:
  /// Constructor for runtime array
  /// @param subtype the type of the array elements
  explicit ArrayType(Type* subtype);
  /// Constructor
  /// @param subtype the type of the array elements
  /// @param size the number of elements in the array
  ArrayType(Type* subtype, uint32_t size);
  /// Move constructor
  ArrayType(ArrayType&&) = default;
  ~ArrayType() override;

  /// @returns true if the type is an array type
  bool IsArray() const override { return true; }
  /// @returns true if this is a runtime array.
  /// i.e. the size is determined at runtime
  bool IsRuntimeArray() const { return size_ == 0; }

  /// @returns the array type
  Type* type() const { return subtype_; }
  /// @returns the array size. Size is 0 for a runtime array
  uint32_t size() const { return size_; }

  /// @returns the name for the type
  std::string type_name() const override {
    assert(subtype_);

    std::string type_name = "__array" + subtype_->type_name();
    if (!IsRuntimeArray())
      type_name += "_" + std::to_string(size_);

    return type_name;
  }

 private:
  Type* subtype_ = nullptr;
  uint32_t size_ = 0;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_ARRAY_TYPE_H_
