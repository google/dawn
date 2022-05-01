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

#include "src/tint/sem/variable.h"

#include <utility>

#include "src/tint/ast/identifier_expression.h"
#include "src/tint/ast/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Variable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::GlobalVariable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::LocalVariable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::Parameter);
TINT_INSTANTIATE_TYPEINFO(tint::sem::VariableUser);

namespace tint::sem {

Variable::Variable(const ast::Variable* declaration,
                   const sem::Type* type,
                   ast::StorageClass storage_class,
                   ast::Access access,
                   Constant constant_value)
    : declaration_(declaration),
      type_(type),
      storage_class_(storage_class),
      access_(access),
      constant_value_(constant_value) {}

Variable::~Variable() = default;

LocalVariable::LocalVariable(const ast::Variable* declaration,
                             const sem::Type* type,
                             ast::StorageClass storage_class,
                             ast::Access access,
                             const sem::Statement* statement,
                             Constant constant_value)
    : Base(declaration, type, storage_class, access, std::move(constant_value)),
      statement_(statement) {}

LocalVariable::~LocalVariable() = default;

GlobalVariable::GlobalVariable(const ast::Variable* declaration,
                               const sem::Type* type,
                               ast::StorageClass storage_class,
                               ast::Access access,
                               Constant constant_value,
                               sem::BindingPoint binding_point)
    : Base(declaration, type, storage_class, access, std::move(constant_value)),
      binding_point_(binding_point) {}

GlobalVariable::~GlobalVariable() = default;

Parameter::Parameter(const ast::Variable* declaration,
                     uint32_t index,
                     const sem::Type* type,
                     ast::StorageClass storage_class,
                     ast::Access access,
                     const ParameterUsage usage /* = ParameterUsage::kNone */)
    : Base(declaration, type, storage_class, access, Constant{}), index_(index), usage_(usage) {}

Parameter::~Parameter() = default;

VariableUser::VariableUser(const ast::IdentifierExpression* declaration,
                           Statement* statement,
                           sem::Variable* variable)
    : Base(declaration,
           variable->Type(),
           statement,
           variable->ConstantValue(),
           /* has_side_effects */ false),
      variable_(variable) {
    auto* type = variable->Type();
    if (type->Is<sem::Pointer>() && variable->Constructor()) {
        source_variable_ = variable->Constructor()->SourceVariable();
    } else {
        source_variable_ = variable;
    }
}

}  // namespace tint::sem
