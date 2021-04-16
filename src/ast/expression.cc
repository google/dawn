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

#include "src/ast/expression.h"

#include "src/sem/expression.h"
#include "src/sem/info.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Expression);

namespace tint {
namespace ast {

Expression::Expression(ProgramID program_id, const Source& source)
    : Base(program_id, source) {}

Expression::Expression(Expression&&) = default;

Expression::~Expression() = default;

std::string Expression::result_type_str(const sem::Info& sem) const {
  auto* sem_expr = sem.Get(this);
  return sem_expr ? sem_expr->Type()->type_name() : "not set";
}

}  // namespace ast
}  // namespace tint
