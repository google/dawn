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

#include "src/transform/remove_unreachable_statements.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/traverse_expressions.h"
#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"
#include "src/utils/map.h"
#include "src/utils/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::RemoveUnreachableStatements);

namespace tint {
namespace transform {

RemoveUnreachableStatements::RemoveUnreachableStatements() = default;

RemoveUnreachableStatements::~RemoveUnreachableStatements() = default;

void RemoveUnreachableStatements::Run(CloneContext& ctx,
                                      const DataMap&,
                                      DataMap&) {
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* stmt = ctx.src->Sem().Get<sem::Statement>(node)) {
      if (!stmt->IsReachable()) {
        RemoveStatement(ctx, stmt->Declaration());
      }
    }
  }

  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
