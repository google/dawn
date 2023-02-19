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
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/type/reference.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Robustness);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Robustness::Config);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

/// PIMPL state for the transform
struct Robustness::State {
    /// Constructor
    /// @param program the source program
    /// @param omitted the omitted address spaces
    State(const Program* program, std::unordered_set<builtin::AddressSpace>&& omitted)
        : src(program), omitted_address_spaces(std::move(omitted)) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        ctx.ReplaceAll([&](const ast::IndexAccessorExpression* expr) { return Transform(expr); });
        ctx.ReplaceAll([&](const ast::CallExpression* expr) { return Transform(expr); });

        ctx.Clone();
        return Program(std::move(b));
    }

  private:
    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};

    /// Set of address spaces to not apply the transform to
    std::unordered_set<builtin::AddressSpace> omitted_address_spaces;

    /// Apply bounds clamping to array, vector and matrix indexing
    /// @param expr the array, vector or matrix index expression
    /// @return the clamped replacement expression, or nullptr if `expr` should be cloned without
    /// changes.
    const ast::IndexAccessorExpression* Transform(const ast::IndexAccessorExpression* expr) {
        auto* sem = src->Sem().Get(expr)->Unwrap()->As<sem::IndexAccessorExpression>();
        auto* ret_type = sem->Type();

        auto* ref = ret_type->As<type::Reference>();
        if (ref && omitted_address_spaces.count(ref->AddressSpace()) != 0) {
            return nullptr;
        }

        // idx return the cloned index expression, as a u32.
        auto idx = [&]() -> const ast::Expression* {
            auto* i = ctx.Clone(expr->index);
            if (sem->Index()->Type()->is_signed_integer_scalar()) {
                return b.Call<u32>(i);  // u32(idx)
            }
            return i;
        };

        auto* clamped_idx = Switch(
            sem->Object()->Type()->UnwrapRef(),  //
            [&](const type::Vector* vec) -> const ast::Expression* {
                if (sem->Index()->ConstantValue()) {
                    // Index and size is constant.
                    // Validation will have rejected any OOB accesses.
                    return nullptr;
                }

                return b.Call("min", idx(), u32(vec->Width() - 1u));
            },
            [&](const type::Matrix* mat) -> const ast::Expression* {
                if (sem->Index()->ConstantValue()) {
                    // Index and size is constant.
                    // Validation will have rejected any OOB accesses.
                    return nullptr;
                }

                return b.Call("min", idx(), u32(mat->columns() - 1u));
            },
            [&](const type::Array* arr) -> const ast::Expression* {
                const ast::Expression* max = nullptr;
                if (arr->Count()->Is<type::RuntimeArrayCount>()) {
                    // Size is unknown until runtime.
                    // Must clamp, even if the index is constant.
                    auto* arr_ptr = b.AddressOf(ctx.Clone(expr->object));
                    max = b.Sub(b.Call("arrayLength", arr_ptr), 1_u);
                } else if (auto count = arr->ConstantCount()) {
                    if (sem->Index()->ConstantValue()) {
                        // Index and size is constant.
                        // Validation will have rejected any OOB accesses.
                        return nullptr;
                    }
                    max = b.Expr(u32(count.value() - 1u));
                } else {
                    // Note: Don't be tempted to use the array override variable as an expression
                    // here, the name might be shadowed!
                    b.Diagnostics().add_error(diag::System::Transform,
                                              type::Array::kErrExpectedConstantCount);
                    return nullptr;
                }

                return b.Call("min", idx(), max);
            },
            [&](Default) {
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled object type in robustness of array index: "
                    << src->FriendlyName(ret_type->UnwrapRef());
                return nullptr;
            });

        if (!clamped_idx) {
            return nullptr;  // Clamping not needed
        }

        auto idx_src = ctx.Clone(expr->source);
        auto* idx_obj = ctx.Clone(expr->object);
        return b.IndexAccessor(idx_src, idx_obj, clamped_idx);
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
        auto* call = src->Sem().Get(expr)->UnwrapMaterialize()->As<sem::Call>();
        auto* call_target = call->Target();
        auto* builtin = call_target->As<sem::Builtin>();
        if (!builtin || !TextureBuiltinNeedsClamping(builtin->Type())) {
            return nullptr;  // No transform, just clone.
        }

        // Indices of the mandatory texture and coords parameters, and the optional
        // array and level parameters.
        auto& signature = builtin->Signature();
        auto texture_idx = signature.IndexOf(sem::ParameterUsage::kTexture);
        auto coords_idx = signature.IndexOf(sem::ParameterUsage::kCoords);
        auto array_idx = signature.IndexOf(sem::ParameterUsage::kArrayIndex);
        auto level_idx = signature.IndexOf(sem::ParameterUsage::kLevel);

        auto* texture_arg = expr->args[static_cast<size_t>(texture_idx)];
        auto* coords_arg = expr->args[static_cast<size_t>(coords_idx)];
        auto* coords_ty = builtin->Parameters()[static_cast<size_t>(coords_idx)]->Type();

        auto width_of = [&](const type::Type* ty) {
            if (auto* vec = ty->As<type::Vector>()) {
                return vec->Width();
            }
            return 1u;
        };
        auto scalar_or_vec_ty = [&](ast::Type scalar, uint32_t width) {
            if (width > 1) {
                return b.ty.vec(scalar, width);
            }
            return scalar;
        };
        auto scalar_or_vec = [&](const ast::Expression* scalar,
                                 uint32_t width) -> const ast::Expression* {
            if (width > 1) {
                return b.Call(b.ty.vec<Infer>(width), scalar);
            }
            return scalar;
        };
        auto cast_to_signed = [&](const ast::Expression* val, uint32_t width) {
            return b.Call(scalar_or_vec_ty(b.ty.i32(), width), val);
        };
        auto cast_to_unsigned = [&](const ast::Expression* val, uint32_t width) {
            return b.Call(scalar_or_vec_ty(b.ty.u32(), width), val);
        };

        // If the level is provided, then we need to clamp this. As the level is
        // used by textureDimensions() and the texture[Load|Store]() calls, we need
        // to clamp both usages.
        // TODO(bclayton): We probably want to place this into a let so that the
        // calculation can be reused. This is fiddly to get right.
        std::function<const ast::Expression*()> level_arg;
        if (level_idx >= 0) {
            level_arg = [&] {
                const auto* arg = expr->args[static_cast<size_t>(level_idx)];
                const auto* target_ty =
                    builtin->Parameters()[static_cast<size_t>(level_idx)]->Type();
                const auto* num_levels = b.Call("textureNumLevels", ctx.Clone(texture_arg));

                // TODO(crbug.com/tint/1526) remove when num_levels returns u32
                num_levels = cast_to_unsigned(num_levels, 1u);

                const auto* unsigned_max = b.Sub(num_levels, 1_a);
                if (target_ty->is_signed_integer_scalar()) {
                    const auto* signed_max = cast_to_signed(unsigned_max, 1u);
                    return b.Call("clamp", ctx.Clone(arg), 0_a, signed_max);
                } else {
                    return b.Call("min", ctx.Clone(arg), unsigned_max);
                }
            };
        }

        // Clamp the coordinates argument
        {
            const auto* target_ty = coords_ty;
            const auto width = width_of(target_ty);
            const auto* texture_dims =
                level_arg ? b.Call("textureDimensions", ctx.Clone(texture_arg), level_arg())
                          : b.Call("textureDimensions", ctx.Clone(texture_arg));

            // TODO(crbug.com/tint/1526) remove when texture_dims returns u32 or vecN<u32>
            texture_dims = cast_to_unsigned(texture_dims, width);

            // texture_dims is u32 or vecN<u32>
            const auto* unsigned_max = b.Sub(texture_dims, scalar_or_vec(b.Expr(1_a), width));
            if (target_ty->is_signed_integer_scalar_or_vector()) {
                const auto* zero = scalar_or_vec(b.Expr(0_a), width);
                const auto* signed_max = cast_to_signed(unsigned_max, width);
                ctx.Replace(coords_arg, b.Call("clamp", ctx.Clone(coords_arg), zero, signed_max));
            } else {
                ctx.Replace(coords_arg, b.Call("min", ctx.Clone(coords_arg), unsigned_max));
            }
        }

        // Clamp the array_index argument, if provided
        if (array_idx >= 0) {
            auto* target_ty = builtin->Parameters()[static_cast<size_t>(array_idx)]->Type();
            auto* arg = expr->args[static_cast<size_t>(array_idx)];
            auto* num_layers = b.Call("textureNumLayers", ctx.Clone(texture_arg));

            // TODO(crbug.com/tint/1526) remove when num_layers returns u32
            num_layers = cast_to_unsigned(num_layers, 1u);

            const auto* unsigned_max = b.Sub(num_layers, 1_a);
            if (target_ty->is_signed_integer_scalar()) {
                const auto* signed_max = cast_to_signed(unsigned_max, 1u);
                ctx.Replace(arg, b.Call("clamp", ctx.Clone(arg), 0_a, signed_max));
            } else {
                ctx.Replace(arg, b.Call("min", ctx.Clone(arg), unsigned_max));
            }
        }

        // Clamp the level argument, if provided
        if (level_idx >= 0) {
            auto* arg = expr->args[static_cast<size_t>(level_idx)];
            ctx.Replace(arg, level_arg ? level_arg() : b.Expr(0_a));
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

Transform::ApplyResult Robustness::Apply(const Program* src,
                                         const DataMap& inputs,
                                         DataMap&) const {
    Config cfg;
    if (auto* cfg_data = inputs.Get<Config>()) {
        cfg = *cfg_data;
    }

    std::unordered_set<builtin::AddressSpace> omitted_address_spaces;
    for (auto sc : cfg.omitted_address_spaces) {
        switch (sc) {
            case AddressSpace::kUniform:
                omitted_address_spaces.insert(builtin::AddressSpace::kUniform);
                break;
            case AddressSpace::kStorage:
                omitted_address_spaces.insert(builtin::AddressSpace::kStorage);
                break;
        }
    }

    return State{src, std::move(omitted_address_spaces)}.Run();
}

}  // namespace tint::transform
