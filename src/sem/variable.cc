// Copyright 2021 The Tint Authors.
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

#include "src/sem/variable.h"

#include "src/ast/identifier_expression.h"
#include "src/ast/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Variable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::VariableUser);

namespace tint {
namespace sem {

Variable::Variable(const ast::Variable* declaration,
                   const sem::Type* type,
                   ast::StorageClass storage_class,
                   ast::AccessControl::Access access_control)
    : declaration_(declaration),
      type_(type),
      storage_class_(storage_class),
      access_control_(access_control),
      is_pipeline_constant_(false) {}

Variable::Variable(const ast::Variable* declaration,
                   const sem::Type* type,
                   uint16_t constant_id)
    : declaration_(declaration),
      type_(type),
      storage_class_(ast::StorageClass::kNone),
      access_control_(ast::AccessControl::kReadWrite),
      is_pipeline_constant_(true),
      constant_id_(constant_id) {}

Variable::~Variable() = default;

VariableUser::VariableUser(ast::IdentifierExpression* declaration,
                           const sem::Type* type,
                           Statement* statement,
                           sem::Variable* variable)
    : Base(declaration, type, statement), variable_(variable) {}

}  // namespace sem
}  // namespace tint
