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

#ifndef SRC_SEM_VECTOR_TYPE_H_
#define SRC_SEM_VECTOR_TYPE_H_

#include <string>

#include "src/sem/type.h"

namespace tint {
namespace sem {

/// A vector type.
class Vector : public Castable<Vector, Type> {
 public:
  /// Constructor
  /// @param subtype the vector element type
  /// @param size the number of elements in the vector
  Vector(Type const* subtype, uint32_t size);
  /// Move constructor
  Vector(Vector&&);
  ~Vector() override;

  /// @returns the type of the vector elements
  Type* type() const { return const_cast<Type*>(subtype_); }
  /// @returns the size of the vector
  uint32_t size() const { return size_; }

  /// @returns the name for th type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

 private:
  Type const* const subtype_;
  uint32_t const size_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_VECTOR_TYPE_H_
