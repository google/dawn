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
#include "src/tint/ast/parameter.h"
#include "src/tint/ast/variable.h"
#include "src/tint/type/pointer.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Variable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::GlobalVariable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::LocalVariable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::Parameter);
TINT_INSTANTIATE_TYPEINFO(tint::sem::VariableUser);

namespace tint::sem {
Variable::Variable(const ast::Variable* declaration,
                   const type::Type* type,
                   EvaluationStage stage,
                   builtin::AddressSpace address_space,
                   builtin::Access access,
                   const constant::Value* constant_value)
    : declaration_(declaration),
      type_(type),
      stage_(stage),
      address_space_(address_space),
      access_(access),
      constant_value_(constant_value) {}

Variable::~Variable() = default;

LocalVariable::LocalVariable(const ast::Variable* declaration,
                             const type::Type* type,
                             EvaluationStage stage,
                             builtin::AddressSpace address_space,
                             builtin::Access access,
                             const sem::Statement* statement,
                             const constant::Value* constant_value)
    : Base(declaration, type, stage, address_space, access, constant_value),
      statement_(statement) {}

LocalVariable::~LocalVariable() = default;

GlobalVariable::GlobalVariable(const ast::Variable* declaration,
                               const type::Type* type,
                               EvaluationStage stage,
                               builtin::AddressSpace address_space,
                               builtin::Access access,
                               const constant::Value* constant_value,
                               std::optional<sem::BindingPoint> binding_point,
                               std::optional<uint32_t> location)
    : Base(declaration, type, stage, address_space, access, constant_value),
      binding_point_(binding_point),
      location_(location) {}

GlobalVariable::~GlobalVariable() = default;

Parameter::Parameter(const ast::Parameter* declaration,
                     uint32_t index,
                     const type::Type* type,
                     builtin::AddressSpace address_space,
                     builtin::Access access,
                     const ParameterUsage usage /* = ParameterUsage::kNone */,
                     std::optional<sem::BindingPoint> binding_point /* = {} */,
                     std::optional<uint32_t> location /* = std::nullopt */)
    : Base(declaration, type, EvaluationStage::kRuntime, address_space, access, nullptr),
      index_(index),
      usage_(usage),
      binding_point_(binding_point),
      location_(location) {}

Parameter::~Parameter() = default;

VariableUser::VariableUser(const ast::IdentifierExpression* declaration,
                           Statement* statement,
                           sem::Variable* variable)
    : Base(declaration,
           variable->Type(),
           variable->Stage(),
           statement,
           variable->ConstantValue(),
           /* has_side_effects */ false),
      variable_(variable) {
    auto* type = variable->Type();
    if (type->Is<type::Pointer>() && variable->Initializer()) {
        root_identifier_ = variable->Initializer()->RootIdentifier();
    } else {
        root_identifier_ = variable;
    }
}

VariableUser::~VariableUser() = default;

}  // namespace tint::sem
