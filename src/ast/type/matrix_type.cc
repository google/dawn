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

bool MatrixType::IsMatrix() const {
  return true;
}

std::string MatrixType::type_name() const {
  return "__mat_" + std::to_string(rows_) + "_" + std::to_string(columns_) +
         subtype_->type_name();
}

MatrixType::~MatrixType() = default;

}  // namespace type
}  // namespace ast
}  // namespace tint
