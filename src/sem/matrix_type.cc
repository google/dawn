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

#include "src/sem/matrix_type.h"

#include "src/program_builder.h"
#include "src/sem/vector_type.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Matrix);

namespace tint {
namespace sem {

Matrix::Matrix(Vector* column_type, uint32_t columns)
    : subtype_(column_type->type()),
      column_type_(column_type),
      rows_(column_type->size()),
      columns_(columns) {
  TINT_ASSERT(rows_ > 1);
  TINT_ASSERT(rows_ < 5);
  TINT_ASSERT(columns_ > 1);
  TINT_ASSERT(columns_ < 5);
}

Matrix::Matrix(Matrix&&) = default;

Matrix::~Matrix() = default;

std::string Matrix::type_name() const {
  return "__mat_" + std::to_string(rows_) + "_" + std::to_string(columns_) +
         subtype_->type_name();
}

std::string Matrix::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  out << "mat" << columns_ << "x" << rows_ << "<"
      << subtype_->FriendlyName(symbols) << ">";
  return out.str();
}

}  // namespace sem
}  // namespace tint
