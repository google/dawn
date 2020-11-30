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

#include "src/ast/type/matrix_type.h"

#include <assert.h>

#include "src/ast/type/array_type.h"
#include "src/ast/type/vector_type.h"

namespace tint {
namespace ast {
namespace type {

MatrixType::MatrixType(Type* subtype, uint32_t rows, uint32_t columns)
    : subtype_(subtype), rows_(rows), columns_(columns) {
  assert(rows > 1);
  assert(rows < 5);
  assert(columns > 1);
  assert(columns < 5);
}

MatrixType::MatrixType(MatrixType&&) = default;

MatrixType::~MatrixType() = default;

std::string MatrixType::type_name() const {
  return "__mat_" + std::to_string(rows_) + "_" + std::to_string(columns_) +
         subtype_->type_name();
}

uint64_t MatrixType::MinBufferBindingSize(MemoryLayout mem_layout) const {
  VectorType vec(subtype_, rows_);
  return (columns_ - 1) * vec.BaseAlignment(mem_layout) +
         vec.MinBufferBindingSize(mem_layout);
}

uint64_t MatrixType::BaseAlignment(MemoryLayout mem_layout) const {
  VectorType vec(subtype_, rows_);
  ArrayType arr(&vec, columns_);
  return arr.BaseAlignment(mem_layout);
}

}  // namespace type
}  // namespace ast
}  // namespace tint
