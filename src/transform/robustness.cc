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

#include "src/transform/robustness.h"

#include <algorithm>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/call.h"
#include "src/sem/expression.h"
#include "src/sem/statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Robustness);

namespace tint {
namespace transform {

Robustness::Robustness() = default;
Robustness::~Robustness() = default;

void Robustness::Run(CloneContext& ctx, const DataMap&, DataMap&) {
  ctx.ReplaceAll([&](ast::ArrayAccessorExpression* expr) {
    return Transform(expr, &ctx);
  });
  ctx.ReplaceAll(
      [&](ast::CallExpression* expr) { return Transform(expr, &ctx); });
  ctx.Clone();
}

// Apply bounds clamping to array, vector and matrix indexing
ast::ArrayAccessorExpression* Robustness::Transform(
    ast::ArrayAccessorExpression* expr,
    CloneContext* ctx) {
  auto* ret_type = ctx->src->Sem().Get(expr->array())->Type()->UnwrapRef();
  if (!ret_type->IsAnyOf<sem::Array, sem::Matrix, sem::Vector>()) {
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
    if (!is_arr) {
      b.Diagnostics().add_error(diag::System::Transform,
                                "invalid 0 sized non-array", expr->source());
      return nullptr;
    }
    // Runtime sized array
    auto* arr = ctx->Clone(expr->array());
    auto* arr_len = b.Call("arrayLength", b.AddressOf(arr));
    auto* limit = b.Sub(arr_len, b.Expr(1u));
    new_idx = b.Call("min", b.Construct<u32>(ctx->Clone(old_idx)), limit);
  } else if (auto* c = old_idx->As<ast::ScalarConstructorExpression>()) {
    // Scalar constructor we can re-write the value to be within bounds.
    auto* lit = c->literal();
    if (auto* sint = lit->As<ast::SintLiteral>()) {
      int32_t max = static_cast<int32_t>(size) - 1;
      new_idx = b.Expr(std::max(std::min(sint->value(), max), 0));
    } else if (auto* uint = lit->As<ast::UintLiteral>()) {
      new_idx = b.Expr(std::min(uint->value(), size - 1));
    } else {
      b.Diagnostics().add_error(diag::System::Transform,
                                "unknown scalar constructor type for accessor",
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
  return b.IndexAccessor(src, arr, new_idx);
}

// Apply bounds clamping textureLoad() and textureStore() coordinates
ast::CallExpression* Robustness::Transform(ast::CallExpression* expr,
                                           CloneContext* ctx) {
  auto* call = ctx->src->Sem().Get(expr);
  auto* call_target = call->Target();
  auto* intrinsic = call_target->As<sem::Intrinsic>();
  if (!intrinsic || (intrinsic->Type() != sem::IntrinsicType::kTextureLoad &&
                     intrinsic->Type() != sem::IntrinsicType::kTextureStore)) {
    return nullptr;  // No transform, just clone.
  }

  // Index of the texture and coords parameters for the intrinsic overload
  auto texture_idx =
      sem::IndexOf(intrinsic->Parameters(), sem::ParameterUsage::kTexture);
  auto coords_idx =
      sem::IndexOf(intrinsic->Parameters(), sem::ParameterUsage::kCoords);

  auto* texture_arg = expr->params()[texture_idx];
  auto* coords_arg = expr->params()[coords_idx];
  auto* coords_ty = intrinsic->Parameters()[coords_idx].type;

  ProgramBuilder& b = *ctx->dst;
  auto* texture_dims = b.Call("textureDimensions", ctx->Clone(texture_arg));
  auto* zero_dims = b.Construct(CreateASTTypeFor(ctx, coords_ty));
  auto* clamped_coords =
      b.Call("clamp", ctx->Clone(coords_arg), zero_dims, texture_dims);

  ctx->Replace(coords_arg, clamped_coords);
  return nullptr;  // Clone, which will use the coords replacement above.
}

}  // namespace transform
}  // namespace tint
