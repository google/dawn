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

#ifndef SRC_TINT_SEM_REFERENCE_H_
#define SRC_TINT_SEM_REFERENCE_H_

#include <string>

#include "src/tint/ast/access.h"
#include "src/tint/ast/storage_class.h"
#include "src/tint/sem/type.h"

namespace tint::sem {

/// A reference type.
class Reference final : public Castable<Reference, Type> {
 public:
  /// Constructor
  /// @param subtype the pointee type
  /// @param storage_class the storage class of the reference
  /// @param access the resolved access control of the reference
  Reference(const Type* subtype,
            ast::StorageClass storage_class,
            ast::Access access);

  /// Move constructor
  Reference(Reference&&);
  ~Reference() override;

  /// @returns a hash of the type.
  size_t Hash() const override;

  /// @param other the other type to compare against
  /// @returns true if the this type is equal to the given type
  bool Equals(const Type& other) const override;

  /// @returns the pointee type
  const Type* StoreType() const { return subtype_; }

  /// @returns the storage class of the reference
  ast::StorageClass StorageClass() const { return storage_class_; }

  /// @returns the resolved access control of the reference.
  ast::Access Access() const { return access_; }

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

 private:
  Type const* const subtype_;
  ast::StorageClass const storage_class_;
  ast::Access const access_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_REFERENCE_H_
