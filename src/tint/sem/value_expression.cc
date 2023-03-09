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

#include "src/tint/sem/value_expression.h"

#include <utility>

#include "src/tint/sem/load.h"
#include "src/tint/sem/materialize.h"
#include "src/tint/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::ValueExpression);

namespace tint::sem {

ValueExpression::ValueExpression(const ast::Expression* declaration,
                                 const type::Type* type,
                                 EvaluationStage stage,
                                 const Statement* statement,
                                 const constant::Value* constant,
                                 bool has_side_effects,
                                 const Variable* root_ident /* = nullptr */)
    : Base(declaration, statement),
      root_identifier_(root_ident),
      type_(type),
      stage_(stage),
      constant_(std::move(constant)),
      has_side_effects_(has_side_effects) {
    TINT_ASSERT(Semantic, type_);
    TINT_ASSERT(Semantic, (constant != nullptr) == (stage == EvaluationStage::kConstant));
    if (constant != nullptr) {
        TINT_ASSERT(Semantic, type_ == constant->Type());
    }
}

ValueExpression::~ValueExpression() = default;

const ValueExpression* ValueExpression::UnwrapMaterialize() const {
    if (auto* m = As<Materialize>()) {
        return m->Expr();
    }
    return this;
}

const ValueExpression* ValueExpression::UnwrapLoad() const {
    if (auto* l = As<Load>()) {
        return l->Reference();
    }
    return this;
}

const ValueExpression* ValueExpression::Unwrap() const {
    return Switch(
        this,  // note: An expression can only be wrapped by a Load or Materialize, not both.
        [&](const Load* load) { return load->Reference(); },
        [&](const Materialize* materialize) { return materialize->Expr(); },
        [&](Default) { return this; });
}

}  // namespace tint::sem
