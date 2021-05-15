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

#include "src/sem/block_statement.h"

#include "src/ast/block_statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::BlockStatement);

namespace tint {
namespace sem {

BlockStatement::BlockStatement(const ast::BlockStatement* declaration,
                               const Statement* parent,
                               Type type)
    : Base(declaration, parent), type_(type) {}

BlockStatement::~BlockStatement() = default;

const BlockStatement* BlockStatement::FindFirstParent(
    BlockStatement::Type ty) const {
  return FindFirstParent(
      [ty](auto* block_info) { return block_info->type_ == ty; });
}

const ast::BlockStatement* BlockStatement::Declaration() const {
  return Base::Declaration()->As<ast::BlockStatement>();
}

void BlockStatement::SetFirstContinue(size_t first_continue) {
  TINT_ASSERT(type_ == Type::kLoop);
  first_continue_ = first_continue;
}

void BlockStatement::AddDecl(ast::Variable* var) {
  decls_.push_back(var);
}

}  // namespace sem
}  // namespace tint
