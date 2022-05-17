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

#include "src/tint/transform/robustness.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Robustness);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Robustness::Config);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

/// State holds the current transform state
struct Robustness::State {
    /// The clone context
    CloneContext& ctx;

    /// Set of storage classes to not apply the transform to
    std::unordered_set<ast::StorageClass> omitted_classes;

    /// Applies the transformation state to `ctx`.
    void Transform() {
        ctx.ReplaceAll([&](const ast::IndexAccessorExpression* expr) { return Transform(expr); });
        ctx.ReplaceAll([&](const ast::CallExpression* expr) { return Transform(expr); });
    }

    /// Apply bounds clamping to array, vector and matrix indexing
    /// @param expr the array, vector or matrix index expression
    /// @return the clamped replacement expression, or nullptr if `expr` should be
    /// cloned without changes.
    const ast::IndexAccessorExpression* Transform(const ast::IndexAccessorExpression* expr) {
        auto* ret_type = ctx.src->Sem().Get(expr->object)->Type();

        auto* ref = ret_type->As<sem::Reference>();
        if (ref && omitted_classes.count(ref->StorageClass()) != 0) {
            return nullptr;
        }

        auto* ret_unwrapped = ret_type->UnwrapRef();

        ProgramBuilder& b = *ctx.dst;

        struct Value {
            const ast::Expression* expr = nullptr;  // If null, then is a constant
            union {
                uint32_t u32 = 0;  // use if is_signed == false
                int32_t i32;       // use if is_signed == true
            };
            bool is_signed = false;
        };

        Value size;              // size of the array, vector or matrix
        size.is_signed = false;  // size is always unsigned
        if (auto* vec = ret_unwrapped->As<sem::Vector>()) {
            size.u32 = vec->Width();

        } else if (auto* arr = ret_unwrapped->As<sem::Array>()) {
            size.u32 = arr->Count();
        } else if (auto* mat = ret_unwrapped->As<sem::Matrix>()) {
            // The row accessor would have been an embedded index accessor and already
            // handled, so we just need to do columns here.
            size.u32 = mat->columns();
        } else {
            return nullptr;
        }

        if (size.u32 == 0) {
            if (!ret_unwrapped->Is<sem::Array>()) {
                b.Diagnostics().add_error(diag::System::Transform, "invalid 0 sized non-array",
                                          expr->source);
                return nullptr;
            }
            // Runtime sized array
            auto* arr = ctx.Clone(expr->object);
            size.expr = b.Call("arrayLength", b.AddressOf(arr));
        }

        // Calculate the maximum possible index value (size-1u)
        // Size must be positive (non-zero), so we can safely subtract 1 here
        // without underflow.
        Value limit;
        limit.is_signed = false;  // Like size, limit is always unsigned.
        if (size.expr) {
            // Dynamic size
            limit.expr = b.Sub(size.expr, 1_u);
        } else {
            // Constant size
            limit.u32 = size.u32 - 1u;
        }

        Value idx;  // index value

        auto* idx_sem = ctx.src->Sem().Get(expr->index);
        auto* idx_ty = idx_sem->Type()->UnwrapRef();
        if (!idx_ty->IsAnyOf<sem::I32, sem::U32>()) {
            TINT_ICE(Transform, b.Diagnostics())
                << "index must be u32 or i32, got " << idx_sem->Type()->TypeInfo().name;
            return nullptr;
        }

        if (auto idx_constant = idx_sem->ConstantValue()) {
            // Constant value index
            if (idx_constant.Type()->Is<sem::I32>()) {
                idx.i32 = static_cast<int32_t>(idx_constant.Element<AInt>(0).value);
                idx.is_signed = true;
            } else if (idx_constant.Type()->Is<sem::U32>()) {
                idx.u32 = static_cast<uint32_t>(idx_constant.Element<AInt>(0).value);
                idx.is_signed = false;
            } else {
                TINT_ICE(Transform, b.Diagnostics()) << "unsupported constant value for accessor "
                                                     << idx_constant.Type()->TypeInfo().name;
                return nullptr;
            }
        } else {
            // Dynamic value index
            idx.expr = ctx.Clone(expr->index);
            idx.is_signed = idx_ty->Is<sem::I32>();
        }

        // Clamp the index so that it cannot exceed limit.
        if (idx.expr || limit.expr) {
            // One of, or both of idx and limit are non-constant.

            // If the index is signed, cast it to a u32 (with clamping if constant).
            if (idx.is_signed) {
                if (idx.expr) {
                    // We don't use a max(idx, 0) here, as that incurs a runtime
                    // performance cost, and if the unsigned value will be clamped by
                    // limit, resulting in a value between [0..limit)
                    idx.expr = b.Construct<u32>(idx.expr);
                    idx.is_signed = false;
                } else {
                    idx.u32 = static_cast<uint32_t>(std::max(idx.i32, 0));
                    idx.is_signed = false;
                }
            }

            // Convert idx and limit to expressions, so we can emit `min(idx, limit)`.
            if (!idx.expr) {
                idx.expr = b.Expr(u32(idx.u32));
            }
            if (!limit.expr) {
                limit.expr = b.Expr(u32(limit.u32));
            }

            // Perform the clamp with `min(idx, limit)`
            idx.expr = b.Call("min", idx.expr, limit.expr);
        } else {
            // Both idx and max are constant.
            if (idx.is_signed) {
                // The index is signed. Calculate limit as signed.
                int32_t signed_limit = static_cast<int32_t>(
                    std::min<uint32_t>(limit.u32, std::numeric_limits<int32_t>::max()));
                idx.i32 = std::max(idx.i32, 0);
                idx.i32 = std::min(idx.i32, signed_limit);
            } else {
                // The index is unsigned.
                idx.u32 = std::min(idx.u32, limit.u32);
            }
        }

        // Convert idx to an expression, so we can emit the new accessor.
        if (!idx.expr) {
            idx.expr = idx.is_signed ? static_cast<const ast::Expression*>(b.Expr(i32(idx.i32)))
                                     : static_cast<const ast::Expression*>(b.Expr(u32(idx.u32)));
        }

        // Clone arguments outside of create() call to have deterministic ordering
        auto src = ctx.Clone(expr->source);
        auto* obj = ctx.Clone(expr->object);
        return b.IndexAccessor(src, obj, idx.expr);
    }

    /// @param type builtin type
    /// @returns true if the given builtin is a texture function that requires
    /// argument clamping,
    bool TextureBuiltinNeedsClamping(sem::BuiltinType type) {
        return type == sem::BuiltinType::kTextureLoad || type == sem::BuiltinType::kTextureStore;
    }

    /// Apply bounds clamping to the coordinates, array index and level arguments
    /// of the `textureLoad()` and `textureStore()` builtins.
    /// @param expr the builtin call expression
    /// @return the clamped replacement call expression, or nullptr if `expr`
    /// should be cloned without changes.
    const ast::CallExpression* Transform(const ast::CallExpression* expr) {
        auto* call = ctx.src->Sem().Get(expr);
        auto* call_target = call->Target();
        auto* builtin = call_target->As<sem::Builtin>();
        if (!builtin || !TextureBuiltinNeedsClamping(builtin->Type())) {
            return nullptr;  // No transform, just clone.
        }

        ProgramBuilder& b = *ctx.dst;

        // Indices of the mandatory texture and coords parameters, and the optional
        // array and level parameters.
        auto& signature = builtin->Signature();
        auto texture_idx = signature.IndexOf(sem::ParameterUsage::kTexture);
        auto coords_idx = signature.IndexOf(sem::ParameterUsage::kCoords);
        auto array_idx = signature.IndexOf(sem::ParameterUsage::kArrayIndex);
        auto level_idx = signature.IndexOf(sem::ParameterUsage::kLevel);

        auto* texture_arg = expr->args[texture_idx];
        auto* coords_arg = expr->args[coords_idx];
        auto* coords_ty = builtin->Parameters()[coords_idx]->Type();

        // If the level is provided, then we need to clamp this. As the level is
        // used by textureDimensions() and the texture[Load|Store]() calls, we need
        // to clamp both usages.
        // TODO(bclayton): We probably want to place this into a let so that the
        // calculation can be reused. This is fiddly to get right.
        std::function<const ast::Expression*()> level_arg;
        if (level_idx >= 0) {
            level_arg = [&] {
                auto* arg = expr->args[level_idx];
                auto* num_levels = b.Call("textureNumLevels", ctx.Clone(texture_arg));
                auto* zero = b.Expr(0_i);
                auto* max = ctx.dst->Sub(num_levels, 1_i);
                auto* clamped = b.Call("clamp", ctx.Clone(arg), zero, max);
                return clamped;
            };
        }

        // Clamp the coordinates argument
        {
            auto* texture_dims =
                level_arg ? b.Call("textureDimensions", ctx.Clone(texture_arg), level_arg())
                          : b.Call("textureDimensions", ctx.Clone(texture_arg));
            auto* zero = b.Construct(CreateASTTypeFor(ctx, coords_ty));
            auto* max =
                ctx.dst->Sub(texture_dims, b.Construct(CreateASTTypeFor(ctx, coords_ty), 1_i));
            auto* clamped_coords = b.Call("clamp", ctx.Clone(coords_arg), zero, max);
            ctx.Replace(coords_arg, clamped_coords);
        }

        // Clamp the array_index argument, if provided
        if (array_idx >= 0) {
            auto* arg = expr->args[array_idx];
            auto* num_layers = b.Call("textureNumLayers", ctx.Clone(texture_arg));
            auto* zero = b.Expr(0_i);
            auto* max = ctx.dst->Sub(num_layers, 1_i);
            auto* clamped = b.Call("clamp", ctx.Clone(arg), zero, max);
            ctx.Replace(arg, clamped);
        }

        // Clamp the level argument, if provided
        if (level_idx >= 0) {
            auto* arg = expr->args[level_idx];
            ctx.Replace(arg, level_arg ? level_arg() : ctx.dst->Expr(0_i));
        }

        return nullptr;  // Clone, which will use the argument replacements above.
    }
};

Robustness::Config::Config() = default;
Robustness::Config::Config(const Config&) = default;
Robustness::Config::~Config() = default;
Robustness::Config& Robustness::Config::operator=(const Config&) = default;

Robustness::Robustness() = default;
Robustness::~Robustness() = default;

void Robustness::Run(CloneContext& ctx, const DataMap& inputs, DataMap&) const {
    Config cfg;
    if (auto* cfg_data = inputs.Get<Config>()) {
        cfg = *cfg_data;
    }

    std::unordered_set<ast::StorageClass> omitted_classes;
    for (auto sc : cfg.omitted_classes) {
        switch (sc) {
            case StorageClass::kUniform:
                omitted_classes.insert(ast::StorageClass::kUniform);
                break;
            case StorageClass::kStorage:
                omitted_classes.insert(ast::StorageClass::kStorage);
                break;
        }
    }

    State state{ctx, std::move(omitted_classes)};

    state.Transform();
    ctx.Clone();
}

}  // namespace tint::transform
