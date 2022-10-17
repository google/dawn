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

#include "src/tint/sem/block_statement.h"

#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/function.h"
#include "src/tint/sem/function.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::BlockStatement);
TINT_INSTANTIATE_TYPEINFO(tint::sem::FunctionBlockStatement);
TINT_INSTANTIATE_TYPEINFO(tint::sem::LoopBlockStatement);

namespace tint::sem {

BlockStatement::BlockStatement(const ast::BlockStatement* declaration,
                               const CompoundStatement* parent,
                               const sem::Function* function)
    : Base(declaration, parent, function) {}

BlockStatement::~BlockStatement() = default;

const ast::BlockStatement* BlockStatement::Declaration() const {
    return Base::Declaration()->As<ast::BlockStatement>();
}

FunctionBlockStatement::FunctionBlockStatement(const sem::Function* function)
    : Base(function->Declaration()->body, nullptr, function) {
    TINT_ASSERT(Semantic, function);
}

FunctionBlockStatement::~FunctionBlockStatement() = default;

LoopBlockStatement::LoopBlockStatement(const ast::BlockStatement* declaration,
                                       const CompoundStatement* parent,
                                       const sem::Function* function)
    : Base(declaration, parent, function) {
    TINT_ASSERT(Semantic, parent);
    TINT_ASSERT(Semantic, function);
}
LoopBlockStatement::~LoopBlockStatement() = default;

void LoopBlockStatement::SetFirstContinue(const ast::ContinueStatement* first_continue,
                                          size_t num_decls) {
    first_continue_ = first_continue;
    num_decls_at_first_continue_ = num_decls;
}

}  // namespace tint::sem
