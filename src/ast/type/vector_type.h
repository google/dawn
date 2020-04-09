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

#ifndef SRC_AST_TYPE_VECTOR_TYPE_H_
#define SRC_AST_TYPE_VECTOR_TYPE_H_

#include <string>

#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// A vector type.
class VectorType : public Type {
 public:
  /// Constructor
  /// @param subtype the vector element type
  /// @param size the number of elements in the vector
  VectorType(Type* subtype, uint32_t size);
  /// Move constructor
  VectorType(VectorType&&);
  ~VectorType() override;

  /// @returns true if the type is a vector type
  bool IsVector() const override;

  /// @returns the type of the vector elements
  Type* type() const { return subtype_; }
  /// @returns the size of the vector
  uint32_t size() const { return size_; }

  /// @returns the name for th type
  std::string type_name() const override;

 private:
  Type* subtype_ = nullptr;
  uint32_t size_ = 2;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_VECTOR_TYPE_H_
