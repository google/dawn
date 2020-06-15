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
  ArrayType(ArrayType&&);
  ~ArrayType() override;

  /// @returns true if the type is an array type
  bool IsArray() const override;
  /// @returns true if this is a runtime array.
  /// i.e. the size is determined at runtime
  bool IsRuntimeArray() const { return size_ == 0; }

  /// Sets the array stride
  /// @param stride the stride to set
  void set_array_stride(uint32_t stride) { array_stride_ = stride; }
  /// @returns the array stride or 0 if none set.
  uint32_t array_stride() const { return array_stride_; }
  /// @returns true if the array has a stride set
  bool has_array_stride() const { return array_stride_ != 0; }

  /// @returns the array type
  Type* type() const { return subtype_; }
  /// @returns the array size. Size is 0 for a runtime array
  uint32_t size() const { return size_; }

  /// @returns the name for the type
  std::string type_name() const override;

 private:
  Type* subtype_ = nullptr;
  uint32_t size_ = 0;
  uint32_t array_stride_ = 0;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_ARRAY_TYPE_H_
