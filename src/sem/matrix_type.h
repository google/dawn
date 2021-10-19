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

#ifndef SRC_SEM_MATRIX_TYPE_H_
#define SRC_SEM_MATRIX_TYPE_H_

#include <string>

#include "src/sem/type.h"

namespace tint {
namespace sem {

// Forward declaration
class Vector;

/// A matrix type
class Matrix : public Castable<Matrix, Type> {
 public:
  /// Constructor
  /// @param column_type the type of a column of the matrix
  /// @param columns the number of columns in the matrix
  Matrix(const Vector* column_type, uint32_t columns);
  /// Move constructor
  Matrix(Matrix&&);
  ~Matrix() override;

  /// @returns the type of the matrix
  const Type* type() const { return subtype_; }
  /// @returns the number of rows in the matrix
  uint32_t rows() const { return rows_; }
  /// @returns the number of columns in the matrix
  uint32_t columns() const { return columns_; }

  /// @returns the column-vector type of the matrix
  const Vector* ColumnType() const { return column_type_; }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

  /// @returns true if constructible as per
  /// https://gpuweb.github.io/gpuweb/wgsl/#constructible-types
  bool IsConstructible() const override;

  /// @returns the size in bytes of the type. This may include tail padding.
  uint32_t Size() const override;

  /// @returns the alignment in bytes of the type. This may include tail
  /// padding.
  uint32_t Align() const override;

  /// @returns the number of bytes between columns of the matrix
  uint32_t ColumnStride() const;

 private:
  const Type* const subtype_;
  const Vector* const column_type_;
  const uint32_t rows_;
  const uint32_t columns_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_MATRIX_TYPE_H_
