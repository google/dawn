// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#include "src/tint/sem/call.h"

#include <utility>
#include <vector>

TINT_INSTANTIATE_TYPEINFO(tint::sem::Call);

namespace tint::sem {

Call::Call(const ast::CallExpression* declaration,
           const CallTarget* target,
           EvaluationStage stage,
           utils::VectorRef<const sem::ValueExpression*> arguments,
           const Statement* statement,
           const constant::Value* constant,
           bool has_side_effects)
    : Base(declaration, target->ReturnType(), stage, statement, constant, has_side_effects),
      target_(target),
      arguments_(std::move(arguments)) {
    // Check that the stage is no earlier than the target supports
    TINT_ASSERT(Semantic,
                (target->Stage() <= stage) || (stage == sem::EvaluationStage::kNotEvaluated));
}

Call::~Call() = default;

}  // namespace tint::sem
