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

#include <algorithm>

#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/identifier.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/statement.h"
#include "src/tint/ast/variable.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Statement);
TINT_INSTANTIATE_TYPEINFO(tint::sem::CompoundStatement);

namespace tint::sem {

Statement::Statement(const ast::Statement* declaration,
                     const CompoundStatement* parent,
                     const sem::Function* function)
    : declaration_(declaration), parent_(parent), function_(function) {}

Statement::~Statement() = default;

const BlockStatement* Statement::Block() const {
    return FindFirstParent<BlockStatement>();
}

CompoundStatement::CompoundStatement(const ast::Statement* declaration,
                                     const CompoundStatement* parent,
                                     const sem::Function* function)
    : Base(declaration, parent, function) {}

CompoundStatement::~CompoundStatement() = default;

void CompoundStatement::AddDecl(const sem::LocalVariable* var) {
    decls_.Add(var->Declaration()->name->symbol, OrderedLocalVariable{decls_.Count(), var});
}

}  // namespace tint::sem
