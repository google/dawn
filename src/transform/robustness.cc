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
#include "src/sem/block_statement.h"
#include "src/sem/call.h"
#include "src/sem/expression.h"
#include "src/sem/statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Robustness);

namespace tint {
namespace transform {

/// State holds the current transform state
struct Robustness::State {
  /// The clone context
  CloneContext& ctx;

  /// Applies the transformation state to `ctx`.
  void Transform() {
    ctx.ReplaceAll(
        [&](ast::ArrayAccessorExpression* expr) { return Transform(expr); });
    ctx.ReplaceAll([&](ast::CallExpression* expr) { return Transform(expr); });
  }

  /// Apply bounds clamping to array, vector and matrix indexing
  /// @param expr the array, vector or matrix index expression
  /// @return the clamped replacement expression, or nullptr if `expr` should be
  /// cloned without changes.
  ast::ArrayAccessorExpression* Transform(ast::ArrayAccessorExpression* expr) {
    auto* ret_type = ctx.src->Sem().Get(expr->array())->Type()->UnwrapRef();
    if (!ret_type->IsAnyOf<sem::Array, sem::Matrix, sem::Vector>()) {
      return nullptr;
    }

    ProgramBuilder& b = *ctx.dst;
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
    b.SetSource(ctx.Clone(old_idx->source()));

    ast::Expression* new_idx = nullptr;

    if (size == 0) {
      if (!is_arr) {
        b.Diagnostics().add_error(diag::System::Transform,
                                  "invalid 0 sized non-array", expr->source());
        return nullptr;
      }
      // Runtime sized array
      auto* arr = ctx.Clone(expr->array());
      auto* arr_len = b.Call("arrayLength", b.AddressOf(arr));
      auto* limit = b.Sub(arr_len, b.Expr(1u));
      new_idx = b.Call("min", b.Construct<u32>(ctx.Clone(old_idx)), limit);
    } else if (auto* c = old_idx->As<ast::ScalarConstructorExpression>()) {
      // Scalar constructor we can re-write the value to be within bounds.
      auto* lit = c->literal();
      if (auto* sint = lit->As<ast::SintLiteral>()) {
        int32_t max = static_cast<int32_t>(size) - 1;
        new_idx = b.Expr(std::max(std::min(sint->value(), max), 0));
      } else if (auto* uint = lit->As<ast::UintLiteral>()) {
        new_idx = b.Expr(std::min(uint->value(), size - 1));
      } else {
        b.Diagnostics().add_error(
            diag::System::Transform,
            "unknown scalar constructor type for accessor", expr->source());
        return nullptr;
      }
    } else {
      auto* cloned_idx = ctx.Clone(old_idx);
      new_idx = b.Call("min", b.Construct<u32>(cloned_idx), b.Expr(size - 1));
    }

    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx.Clone(expr->source());
    auto* arr = ctx.Clone(expr->array());
    return b.IndexAccessor(src, arr, new_idx);
  }

  /// @param type intrinsic type
  /// @returns true if the given intrinsic is a texture function that requires
  /// argument clamping,
  bool TextureIntrinsicNeedsClamping(sem::IntrinsicType type) {
    return type == sem::IntrinsicType::kTextureLoad ||
           type == sem::IntrinsicType::kTextureStore;
  }

  /// Apply bounds clamping to the coordinates, array index and level arguments
  /// of the `textureLoad()` and `textureStore()` intrinsics.
  /// @param expr the intrinsic call expression
  /// @return the clamped replacement call expression, or nullptr if `expr`
  /// should be cloned without changes.
  ast::CallExpression* Transform(ast::CallExpression* expr) {
    auto* call = ctx.src->Sem().Get(expr);
    auto* call_target = call->Target();
    auto* intrinsic = call_target->As<sem::Intrinsic>();
    if (!intrinsic || !TextureIntrinsicNeedsClamping(intrinsic->Type())) {
      return nullptr;  // No transform, just clone.
    }

    ProgramBuilder& b = *ctx.dst;

    // Indices of the mandatory texture and coords parameters, and the optional
    // array and level parameters.
    auto texture_idx =
        sem::IndexOf(intrinsic->Parameters(), sem::ParameterUsage::kTexture);
    auto coords_idx =
        sem::IndexOf(intrinsic->Parameters(), sem::ParameterUsage::kCoords);
    auto array_idx =
        sem::IndexOf(intrinsic->Parameters(), sem::ParameterUsage::kArrayIndex);
    auto level_idx =
        sem::IndexOf(intrinsic->Parameters(), sem::ParameterUsage::kLevel);

    auto* texture_arg = expr->params()[texture_idx];
    auto* coords_arg = expr->params()[coords_idx];
    auto* coords_ty = intrinsic->Parameters()[coords_idx].type;

    // If the level is provided, then we need to clamp this. As the level is
    // used by textureDimensions() and the texture[Load|Store]() calls, we need
    // to clamp both usages.
    // TODO(bclayton): We probably want to place this into a let so that the
    // calculation can be reused. This is fiddly to get right.
    std::function<ast::Expression*()> level_arg;
    if (level_idx >= 0) {
      level_arg = [&] {
        auto* arg = expr->params()[level_idx];
        auto* num_levels = b.Call("textureNumLevels", ctx.Clone(texture_arg));
        auto* zero = b.Expr(0);
        auto* max = ctx.dst->Sub(num_levels, 1);
        auto* clamped = b.Call("clamp", ctx.Clone(arg), zero, max);
        return clamped;
      };
    }

    // Clamp the coordinates argument
    {
      auto* texture_dims =
          level_arg
              ? b.Call("textureDimensions", ctx.Clone(texture_arg), level_arg())
              : b.Call("textureDimensions", ctx.Clone(texture_arg));
      auto* zero = b.Construct(CreateASTTypeFor(ctx, coords_ty));
      auto* max = ctx.dst->Sub(
          texture_dims, b.Construct(CreateASTTypeFor(ctx, coords_ty), 1));
      auto* clamped_coords = b.Call("clamp", ctx.Clone(coords_arg), zero, max);
      ctx.Replace(coords_arg, clamped_coords);
    }

    // Clamp the array_index argument, if provided
    if (array_idx >= 0) {
      auto* arg = expr->params()[array_idx];
      auto* num_layers = b.Call("textureNumLayers", ctx.Clone(texture_arg));
      auto* zero = b.Expr(0);
      auto* max = ctx.dst->Sub(num_layers, 1);
      auto* clamped = b.Call("clamp", ctx.Clone(arg), zero, max);
      ctx.Replace(arg, clamped);
    }

    // Clamp the level argument, if provided
    if (level_idx >= 0) {
      auto* arg = expr->params()[level_idx];
      ctx.Replace(arg, level_arg ? level_arg() : ctx.dst->Expr(0));
    }

    return nullptr;  // Clone, which will use the argument replacements above.
  }
};

Robustness::Robustness() = default;
Robustness::~Robustness() = default;

void Robustness::Run(CloneContext& ctx, const DataMap&, DataMap&) {
  State state{ctx};
  state.Transform();
  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
