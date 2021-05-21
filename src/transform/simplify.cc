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

#include "src/transform/simplify.h"

#include <memory>
#include <unordered_map>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"
#include "src/utils/scoped_assignment.h"

namespace tint {
namespace transform {

Simplify::Simplify() = default;

Simplify::~Simplify() = default;

Output Simplify::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  ctx.ReplaceAll([&](ast::Expression* expr) -> ast::Expression* {
    if (auto* outer = expr->As<ast::UnaryOpExpression>()) {
      if (auto* inner = outer->expr()->As<ast::UnaryOpExpression>()) {
        if (outer->op() == ast::UnaryOp::kAddressOf &&
            inner->op() == ast::UnaryOp::kIndirection) {
          // &(*(expr)) => expr
          return ctx.Clone(inner->expr());
        }
        if (outer->op() == ast::UnaryOp::kIndirection &&
            inner->op() == ast::UnaryOp::kAddressOf) {
          // *(&(expr)) => expr
          return ctx.Clone(inner->expr());
        }
      }
    }
    return nullptr;
  });

  ctx.Clone();

  return Output(Program(std::move(out)));
}

}  // namespace transform
}  // namespace tint
