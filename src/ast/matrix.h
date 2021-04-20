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

#ifndef SRC_AST_MATRIX_H_
#define SRC_AST_MATRIX_H_

#include <string>

#include "src/ast/type.h"

namespace tint {
namespace ast {

/// A matrix type
class Matrix : public Castable<Matrix, Type> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this node
  /// @param subtype type matrix type
  /// @param rows the number of rows in the matrix
  /// @param columns the number of columns in the matrix
  Matrix(ProgramID program_id,
         const Source& source,
         Type* subtype,
         uint32_t rows,
         uint32_t columns);
  /// Move constructor
  Matrix(Matrix&&);
  ~Matrix() override;

  /// @returns the type of the matrix
  Type* type() const { return subtype_; }
  /// @returns the number of rows in the matrix
  uint32_t rows() const { return rows_; }
  /// @returns the number of columns in the matrix
  uint32_t columns() const { return columns_; }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  Matrix* Clone(CloneContext* ctx) const override;

 private:
  Type* const subtype_;
  uint32_t const rows_;
  uint32_t const columns_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_MATRIX_H_
