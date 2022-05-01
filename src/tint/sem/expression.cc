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

#include "src/tint/sem/expression.h"

#include <utility>

TINT_INSTANTIATE_TYPEINFO(tint::sem::Expression);

namespace tint::sem {

Expression::Expression(const ast::Expression* declaration,
                       const sem::Type* type,
                       const Statement* statement,
                       Constant constant,
                       bool has_side_effects,
                       const Variable* source_var /* = nullptr */)
    : declaration_(declaration),
      source_variable_(source_var),
      type_(type),
      statement_(statement),
      constant_(std::move(constant)),
      has_side_effects_(has_side_effects) {
    TINT_ASSERT(Semantic, type_);
}

Expression::~Expression() = default;

}  // namespace tint::sem
