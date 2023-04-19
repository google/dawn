// Copyright 2022 The Tint Authors.
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

#include "src/tint/type/matrix.h"

#include "src/tint/debug.h"
#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/type/manager.h"
#include "src/tint/type/vector.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Matrix);

namespace tint::type {

Matrix::Matrix(const Vector* column_type, uint32_t columns)
    : Base(utils::Hash(utils::TypeInfo::Of<Vector>().full_hashcode, columns, column_type),
           type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }),
      subtype_(column_type->type()),
      column_type_(column_type),
      rows_(column_type->Width()),
      columns_(columns) {
    TINT_ASSERT(AST, rows_ > 1);
    TINT_ASSERT(AST, rows_ < 5);
    TINT_ASSERT(AST, columns_ > 1);
    TINT_ASSERT(AST, columns_ < 5);
}

Matrix::~Matrix() = default;

bool Matrix::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<Matrix>()) {
        return v->rows_ == rows_ && v->columns_ == columns_ && v->column_type_ == column_type_;
    }
    return false;
}

std::string Matrix::FriendlyName() const {
    utils::StringStream out;
    out << "mat" << columns_ << "x" << rows_ << "<" << subtype_->FriendlyName() << ">";
    return out.str();
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

Matrix* Matrix::Clone(CloneContext& ctx) const {
    auto* col_ty = column_type_->Clone(ctx);
    return ctx.dst.mgr->Get<Matrix>(col_ty, columns_);
}

}  // namespace tint::type
