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

#include "src/tint/sem/loop_statement.h"

#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/loop_statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::LoopStatement);
TINT_INSTANTIATE_TYPEINFO(tint::sem::LoopContinuingBlockStatement);

namespace tint::sem {

LoopStatement::LoopStatement(const ast::LoopStatement* declaration,
                             const CompoundStatement* parent,
                             const sem::Function* function)
    : Base(declaration, parent, function) {
    TINT_ASSERT(Semantic, parent);
    TINT_ASSERT(Semantic, function);
}

LoopStatement::~LoopStatement() = default;

LoopContinuingBlockStatement::LoopContinuingBlockStatement(const ast::BlockStatement* declaration,
                                                           const CompoundStatement* parent,
                                                           const sem::Function* function)
    : Base(declaration, parent, function) {
    TINT_ASSERT(Semantic, parent);
    TINT_ASSERT(Semantic, function);
}
LoopContinuingBlockStatement::~LoopContinuingBlockStatement() = default;

const ast::BlockStatement* LoopContinuingBlockStatement::Declaration() const {
    return static_cast<const ast::BlockStatement*>(Base::Declaration());
}

}  // namespace tint::sem
