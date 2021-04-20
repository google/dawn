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

#include "src/ast/matrix.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Matrix);

namespace tint {
namespace ast {

Matrix::Matrix(ProgramID program_id,
               const Source& source,
               Type* subtype,
               uint32_t rows,
               uint32_t columns)
    : Base(program_id, source),
      subtype_(subtype),
      rows_(rows),
      columns_(columns) {
  TINT_ASSERT(rows > 1);
  TINT_ASSERT(rows < 5);
  TINT_ASSERT(columns > 1);
  TINT_ASSERT(columns < 5);
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

Matrix* Matrix::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* ty = ctx->Clone(type());
  return ctx->dst->create<Matrix>(src, ty, rows_, columns_);
}

}  // namespace ast
}  // namespace tint
