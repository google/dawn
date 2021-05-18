// Copyright 2020 The Tint Authors.
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

#include "src/transform/bound_array_accessors.h"

#include <algorithm>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/expression.h"

namespace tint {
namespace transform {

BoundArrayAccessors::BoundArrayAccessors() = default;
BoundArrayAccessors::~BoundArrayAccessors() = default;

Output BoundArrayAccessors::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);
  ctx.ReplaceAll([&](ast::ArrayAccessorExpression* expr) {
    return Transform(expr, &ctx);
  });
  ctx.Clone();
  return Output(Program(std::move(out)));
}

ast::ArrayAccessorExpression* BoundArrayAccessors::Transform(
    ast::ArrayAccessorExpression* expr,
    CloneContext* ctx) {
  auto& diags = ctx->dst->Diagnostics();

  auto* ret_type = ctx->src->Sem().Get(expr->array())->Type()->UnwrapRef();
  if (!ret_type->Is<sem::Array>() && !ret_type->Is<sem::Matrix>() &&
      !ret_type->Is<sem::Vector>()) {
    return nullptr;
  }

  ProgramBuilder& b = *ctx->dst;
  using u32 = ProgramBuilder::u32;

  uint32_t size = 0;
  bool is_vec = ret_type->Is<sem::Vector>();
  bool is_arr = ret_type->Is<sem::Array>();
  if (is_vec || is_arr) {
    size = is_vec ? ret_type->As<sem::Vector>()->size()
                  : ret_type->As<sem::Array>()->Count();
  } else {
    // The row accessor would have been an embedded array accessor and already
    // handled, so we just need to do columns here.
    size = ret_type->As<sem::Matrix>()->columns();
  }

  auto* const old_idx = expr->idx_expr();
  b.SetSource(ctx->Clone(old_idx->source()));

  ast::Expression* new_idx = nullptr;

  if (size == 0) {
    if (is_arr) {
      auto* arr = ctx->Clone(expr->array());
      auto* arr_len = b.Call("arrayLength", arr);
      auto* limit = b.Sub(arr_len, b.Expr(1u));
      new_idx = b.Call("min", b.Construct<u32>(ctx->Clone(old_idx)), limit);
    } else {
      diags.add_error("invalid 0 size", expr->source());
      return nullptr;
    }
  } else if (auto* c = old_idx->As<ast::ScalarConstructorExpression>()) {
    // Scalar constructor we can re-write the value to be within bounds.
    auto* lit = c->literal();
    if (auto* sint = lit->As<ast::SintLiteral>()) {
      int32_t max = static_cast<int32_t>(size) - 1;
      new_idx = b.Expr(std::max(std::min(sint->value(), max), 0));
    } else if (auto* uint = lit->As<ast::UintLiteral>()) {
      new_idx = b.Expr(std::min(uint->value(), size - 1));
    } else {
      diags.add_error("unknown scalar constructor type for accessor",
                      expr->source());
      return nullptr;
    }
  } else {
    auto* cloned_idx = ctx->Clone(old_idx);
    new_idx = b.Call("min", b.Construct<u32>(cloned_idx), b.Expr(size - 1));
  }

  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(expr->source());
  auto* arr = ctx->Clone(expr->array());
  return b.create<ast::ArrayAccessorExpression>(src, arr, new_idx);
}

}  // namespace transform
}  // namespace tint
