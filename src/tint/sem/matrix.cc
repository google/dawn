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

#include "src/tint/sem/matrix.h"

#include "src/tint/program_builder.h"
#include "src/tint/sem/vector.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Matrix);

namespace tint::sem {

Matrix::Matrix(const Vector* column_type, uint32_t columns)
    : subtype_(column_type->type()),
      column_type_(column_type),
      rows_(column_type->Width()),
      columns_(columns) {
    TINT_ASSERT(AST, rows_ > 1);
    TINT_ASSERT(AST, rows_ < 5);
    TINT_ASSERT(AST, columns_ > 1);
    TINT_ASSERT(AST, columns_ < 5);
}

Matrix::Matrix(Matrix&&) = default;

Matrix::~Matrix() = default;

size_t Matrix::Hash() const {
    return utils::Hash(TypeInfo::Of<Vector>().full_hashcode, rows_, columns_, column_type_);
}

bool Matrix::Equals(const Type& other) const {
    if (auto* v = other.As<Matrix>()) {
        return v->rows_ == rows_ && v->columns_ == columns_ && v->column_type_ == column_type_;
    }
    return false;
}

std::string Matrix::FriendlyName(const SymbolTable& symbols) const {
    std::ostringstream out;
    out << "mat" << columns_ << "x" << rows_ << "<" << subtype_->FriendlyName(symbols) << ">";
    return out.str();
}

bool Matrix::IsConstructible() const {
    return true;
}

uint32_t Matrix::Size() const {
    return column_type_->Align() * columns();
}

uint32_t Matrix::Align() const {
    return column_type_->Align();
}

uint32_t Matrix::ColumnStride() const {
    return column_type_->Align();
}

}  // namespace tint::sem
