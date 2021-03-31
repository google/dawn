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

#include "src/ast/block_statement.h"
#include "src/debug.h"
#include "src/semantic/statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::semantic::Statement);

namespace tint {
namespace semantic {

Statement::Statement(const ast::Statement* declaration,
                     const ast::BlockStatement* block)
    : declaration_(declaration), block_(block) {
#ifndef NDEBUG
  if (block) {
    auto& stmts = block->statements();
    TINT_ASSERT(std::find(stmts.begin(), stmts.end(), declaration) !=
                stmts.end());
  }
#endif  //  NDEBUG
}

}  // namespace semantic
}  // namespace tint
