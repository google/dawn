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

#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"

#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"

#include <utility>

TINT_INSTANTIATE_TYPEINFO(tint::sem::IndexAccessorExpression);

namespace tint::sem {

IndexAccessorExpression::IndexAccessorExpression(const ast::IndexAccessorExpression* declaration,
                                                 const type::Type* type,
                                                 core::EvaluationStage stage,
                                                 const ValueExpression* object,
                                                 const ValueExpression* index,
                                                 const Statement* statement,
                                                 const constant::Value* constant,
                                                 bool has_side_effects,
                                                 const Variable* root_ident /* = nullptr */)
    : Base(declaration, type, stage, object, statement, constant, has_side_effects, root_ident),
      index_(index) {}

IndexAccessorExpression::~IndexAccessorExpression() = default;

}  // namespace tint::sem
