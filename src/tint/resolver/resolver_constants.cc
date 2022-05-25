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

#include "src/tint/resolver/resolver.h"

#include <optional>

#include "src/tint/sem/abstract_float.h"
#include "src/tint/sem/abstract_int.h"
#include "src/tint/sem/constant.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/transform.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {

namespace {

/// Converts all the element values of `in` to the type `T`.
/// @param elements_in the vector of elements to be converted
/// @returns the elements converted to type T.
template <typename T, typename ELEMENTS_IN>
sem::Constant::Elements Convert(const ELEMENTS_IN& elements_in) {
    TINT_BEGIN_DISABLE_WARNING_UNREACHABLE_CODE();

    using E = UnwrapNumber<T>;
    return utils::Transform(elements_in, [&](auto value_in) {
        if constexpr (std::is_same_v<E, bool>) {
            return AInt(value_in != 0);
        }

        E converted = static_cast<E>(value_in);
        if constexpr (IsFloatingPoint<E>) {
            return AFloat(converted);
        } else {
            return AInt(converted);
        }
    });

    TINT_END_DISABLE_WARNING_UNREACHABLE_CODE();
}

/// Converts and returns all the element values of `in` to the semantic type `el_ty`.
/// @param in the constant to convert
/// @param el_ty the target element type
/// @returns the elements converted to `type`
sem::Constant::Elements Convert(const sem::Constant::Elements& in, const sem::Type* el_ty) {
    return std::visit(
        [&](auto&& v) {
            return Switch(
                el_ty,  //
                [&](const sem::AbstractInt*) { return Convert<AInt>(v); },
                [&](const sem::AbstractFloat*) { return Convert<AFloat>(v); },
                [&](const sem::I32*) { return Convert<i32>(v); },
                [&](const sem::U32*) { return Convert<u32>(v); },
                [&](const sem::F32*) { return Convert<f32>(v); },
                [&](const sem::F16*) { return Convert<f16>(v); },
                [&](const sem::Bool*) { return Convert<bool>(v); },
                [&](Default) -> sem::Constant::Elements {
                    diag::List diags;
                    TINT_UNREACHABLE(Semantic, diags)
                        << "invalid element type " << el_ty->TypeInfo().name;
                    return {};
                });
        },
        in);
}

}  // namespace

sem::Constant Resolver::EvaluateConstantValue(const ast::Expression* expr, const sem::Type* type) {
    if (auto* e = expr->As<ast::LiteralExpression>()) {
        return EvaluateConstantValue(e, type);
    }
    if (auto* e = expr->As<ast::CallExpression>()) {
        return EvaluateConstantValue(e, type);
    }
    return {};
}

sem::Constant Resolver::EvaluateConstantValue(const ast::LiteralExpression* literal,
                                              const sem::Type* type) {
    return Switch(
        literal,
        [&](const ast::IntLiteralExpression* lit) {
            return sem::Constant{type, {AInt(lit->value)}};
        },
        [&](const ast::FloatLiteralExpression* lit) {
            return sem::Constant{type, {AFloat(lit->value)}};
        },
        [&](const ast::BoolLiteralExpression* lit) {
            return sem::Constant{type, {AInt(lit->value ? 1 : 0)}};
        });
}

sem::Constant Resolver::EvaluateConstantValue(const ast::CallExpression* call,
                                              const sem::Type* ty) {
    uint32_t result_size = 0;
    auto* el_ty = sem::Type::ElementOf(ty, &result_size);
    if (!el_ty) {
        return {};
    }

    // ElementOf() will also return the element type of array, which we do not support.
    if (ty->Is<sem::Array>()) {
        return {};
    }

    // For zero value init, return 0s
    if (call->args.empty()) {
        return Switch(
            el_ty,
            [&](const sem::AbstractInt*) {
                return sem::Constant(ty, std::vector(result_size, AInt(0)));
            },
            [&](const sem::AbstractFloat*) {
                return sem::Constant(ty, std::vector(result_size, AFloat(0)));
            },
            [&](const sem::I32*) { return sem::Constant(ty, std::vector(result_size, AInt(0))); },
            [&](const sem::U32*) { return sem::Constant(ty, std::vector(result_size, AInt(0))); },
            [&](const sem::F32*) { return sem::Constant(ty, std::vector(result_size, AFloat(0))); },
            [&](const sem::F16*) { return sem::Constant(ty, std::vector(result_size, AFloat(0))); },
            [&](const sem::Bool*) { return sem::Constant(ty, std::vector(result_size, AInt(0))); });
    }

    // Build value for type_ctor from each child value by converting to type_ctor's type.
    std::optional<sem::Constant::Elements> elements;
    for (auto* expr : call->args) {
        auto* arg = builder_->Sem().Get(expr);
        if (!arg) {
            return {};
        }
        auto value = arg->ConstantValue();
        if (!value) {
            return {};
        }

        // Convert the elements to the desired type.
        auto converted = Convert(value.GetElements(), el_ty);

        if (elements.has_value()) {
            // Append the converted vector to elements
            std::visit(
                [&](auto&& dst) {
                    using VEC_TY = std::decay_t<decltype(dst)>;
                    const auto& src = std::get<VEC_TY>(converted);
                    dst.insert(dst.end(), src.begin(), src.end());
                },
                elements.value());
        } else {
            elements = std::move(converted);
        }
    }

    // Splat single-value initializers
    std::visit(
        [&](auto&& v) {
            if (v.size() == 1) {
                for (uint32_t i = 0; i < result_size - 1; ++i) {
                    v.emplace_back(v[0]);
                }
            }
        },
        elements.value());

    return sem::Constant(ty, std::move(elements.value()));
}

sem::Constant Resolver::ConvertValue(const sem::Constant& value, const sem::Type* ty) {
    if (value.Type() == ty) {
        return value;
    }

    auto* el_ty = sem::Type::ElementOf(ty);
    if (el_ty == nullptr) {
        return {};
    }
    if (value.ElementType() == el_ty) {
        return sem::Constant(ty, value.GetElements());
    }

    return sem::Constant(ty, Convert(value.GetElements(), el_ty));
}

}  // namespace tint::resolver
