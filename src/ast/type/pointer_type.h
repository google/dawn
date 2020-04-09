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

#ifndef SRC_AST_TYPE_POINTER_TYPE_H_
#define SRC_AST_TYPE_POINTER_TYPE_H_

#include <sstream>
#include <string>

#include "src/ast/storage_class.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// A pointer type.
class PointerType : public Type {
 public:
  /// Construtor
  /// @param subtype the pointee type
  /// @param storage_class the storage class of the pointer
  explicit PointerType(Type* subtype, StorageClass storage_class);
  /// Move constructor
  PointerType(PointerType&&) = default;
  ~PointerType() override;

  /// @returns true if the type is a pointer type
  bool IsPointer() const override;

  /// @returns the pointee type
  Type* type() const { return subtype_; }
  /// @returns the storage class of the pointer
  StorageClass storage_class() const { return storage_class_; }

  /// @returns the name for this type
  std::string type_name() const override;

 private:
  Type* subtype_;
  StorageClass storage_class_;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_POINTER_TYPE_H_
