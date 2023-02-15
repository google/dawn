// Copyright 2022 The Tint Authors.
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

#include "src/tint/sem/while_statement.h"

#include "src/tint/ast/while_statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::WhileStatement);

namespace tint::sem {

WhileStatement::WhileStatement(const ast::WhileStatement* declaration,
                               const CompoundStatement* parent,
                               const sem::Function* function)
    : Base(declaration, parent, function) {}

WhileStatement::~WhileStatement() = default;

const ast::WhileStatement* WhileStatement::Declaration() const {
    return static_cast<const ast::WhileStatement*>(Base::Declaration());
}

}  // namespace tint::sem
