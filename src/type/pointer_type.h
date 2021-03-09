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

#ifndef SRC_TYPE_POINTER_TYPE_H_
#define SRC_TYPE_POINTER_TYPE_H_

#include <string>

#include "src/ast/storage_class.h"
#include "src/type/type.h"

namespace tint {
namespace type {

/// A pointer type.
class Pointer : public Castable<Pointer, Type> {
 public:
  /// Construtor
  /// @param subtype the pointee type
  /// @param storage_class the storage class of the pointer
  explicit Pointer(Type* subtype, ast::StorageClass storage_class);
  /// Move constructor
  Pointer(Pointer&&);
  ~Pointer() override;

  /// @returns the pointee type
  Type* type() const { return subtype_; }
  /// @returns the storage class of the pointer
  ast::StorageClass storage_class() const { return storage_class_; }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  Pointer* Clone(CloneContext* ctx) const override;

 private:
  Type* const subtype_;
  ast::StorageClass const storage_class_;
};

}  // namespace type
}  // namespace tint

#endif  // SRC_TYPE_POINTER_TYPE_H_
