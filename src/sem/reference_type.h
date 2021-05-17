// Copyright 2021 The Tint Authors.
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

#ifndef SRC_SEM_REFERENCE_TYPE_H_
#define SRC_SEM_REFERENCE_TYPE_H_

#include <string>

#include "src/ast/storage_class.h"
#include "src/sem/type.h"

namespace tint {
namespace sem {

/// A reference type.
class Reference : public Castable<Reference, Type> {
 public:
  /// Constructor
  /// @param subtype the pointee type
  /// @param storage_class the storage class of the reference
  Reference(const Type* subtype, ast::StorageClass storage_class);
  /// Move constructor
  Reference(Reference&&);
  ~Reference() override;

  /// @returns the pointee type
  const Type* StoreType() const { return subtype_; }
  /// @returns the storage class of the reference
  ast::StorageClass StorageClass() const { return storage_class_; }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

 private:
  Type const* const subtype_;
  ast::StorageClass const storage_class_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_REFERENCE_TYPE_H_
