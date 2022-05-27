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

#include <cmath>
// TODO(https://crbug.com/dawn/1379) Update cpplint and remove NOLINT
#include <optional>  // NOLINT(build/include_order))

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

/// Converts and returns all the element values of `in` to the type `T`, using the converter
/// function `CONVERTER`.
/// @param elements_in the vector of elements to be converted
/// @param converter a function-like with the signature `void(TO&, FROM)`
/// @returns the elements converted to type T.
template <typename T, typename ELEMENTS_IN, typename CONVERTER>
sem::Constant::Elements Transform(const ELEMENTS_IN& elements_in, CONVERTER&& converter) {
    TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);

    return utils::Transform(elements_in, [&](auto value_in) {
        if constexpr (std::is_same_v<UnwrapNumber<T>, bool>) {
            return AInt(value_in != 0);
        } else {
            T converted{};
            converter(converted, value_in);
            if constexpr (IsFloatingPoint<UnwrapNumber<T>>) {
                return AFloat(converted);
            } else {
                return AInt(converted);
            }
        }
    });

    TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);
}

/// Converts and returns all the element values of `in` to the semantic type `el_ty`, using the
/// converter function `CONVERTER`.
/// @param in the constant to convert
/// @param el_ty the target element type
/// @param converter a function-like with the signature `void(TO&, FROM)`
/// @returns the elements converted to `el_ty`
template <typename CONVERTER>
sem::Constant::Elements Transform(const sem::Constant::Elements& in,
                                  const sem::Type* el_ty,
                                  CONVERTER&& converter) {
    return std::visit(
        [&](auto&& v) {
            return Switch(
                el_ty,  //
                [&](const sem::AbstractInt*) { return Transform<AInt>(v, converter); },
                [&](const sem::AbstractFloat*) { return Transform<AFloat>(v, converter); },
                [&](const sem::I32*) { return Transform<i32>(v, converter); },
                [&](const sem::U32*) { return Transform<u32>(v, converter); },
                [&](const sem::F32*) { return Transform<f32>(v, converter); },
                [&](const sem::F16*) { return Transform<f16>(v, converter); },
                [&](const sem::Bool*) { return Transform<bool>(v, converter); },
                [&](Default) -> sem::Constant::Elements {
                    diag::List diags;
                    TINT_UNREACHABLE(Semantic, diags)
                        << "invalid element type " << el_ty->TypeInfo().name;
                    return {};
                });
        },
        in);
}

/// Converts and returns all the elements in `in` to the type `el_ty`.
/// If the value does not fit in the target type, and:
///  * the target type is an integer type, then the resulting value will be clamped to the integer's
///    highest or lowest value.
///  * the target type is an float type, then the resulting value will be either positive or
///    negative infinity, based on the sign of the input value.
/// @param in the input elements
/// @param el_ty the target element type
/// @returns the elements converted to `el_ty`
sem::Constant::Elements ConvertElements(const sem::Constant::Elements& in, const sem::Type* el_ty) {
    return Transform(in, el_ty, [](auto& el_out, auto el_in) {
        using OUT = std::decay_t<decltype(el_out)>;
        if (auto conv = CheckedConvert<OUT>(el_in)) {
            el_out = conv.Get();
        } else {
            constexpr auto kInf = std::numeric_limits<double>::infinity();
            switch (conv.Failure()) {
                case ConversionFailure::kExceedsNegativeLimit:
                    el_out = IsFloatingPoint<UnwrapNumber<OUT>> ? OUT(-kInf) : OUT::kLowest;
                    break;
                case ConversionFailure::kExceedsPositiveLimit:
                    el_out = IsFloatingPoint<UnwrapNumber<OUT>> ? OUT(kInf) : OUT::kHighest;
                    break;
            }
        }
    });
}

/// Converts and returns all the elements in `in` to the type `el_ty`, by performing a
/// `CheckedConvert` on each element value. A single error diagnostic will be raised if an element
/// value cannot be represented by the target type.
/// @param in the input elements
/// @param el_ty the target element type
/// @returns the elements converted to `el_ty`, or a Failure if some elements could not be
/// represented by the target type.
utils::Result<sem::Constant::Elements> MaterializeElements(const sem::Constant::Elements& in,
                                                           const sem::Type* el_ty,
                                                           ProgramBuilder& builder,
                                                           Source source) {
    std::optional<std::string> failure;

    auto out = Transform(in, el_ty, [&](auto& el_out, auto el_in) {
        using OUT = std::decay_t<decltype(el_out)>;
        if (auto conv = CheckedConvert<OUT>(el_in)) {
            el_out = conv.Get();
        } else if (!failure.has_value()) {
            std::stringstream ss;
            ss << "value " << el_in << " cannot be represented as ";
            ss << "'" << builder.FriendlyName(el_ty) << "'";
            failure = ss.str();
        }
    });

    if (failure.has_value()) {
        builder.Diagnostics().add_error(diag::System::Resolver, std::move(failure.value()), source);
        return utils::Failure;
    }

    return out;
}

}  // namespace

utils::Result<sem::Constant> Resolver::EvaluateConstantValue(const ast::Expression* expr,
                                                             const sem::Type* type) {
    if (auto* e = expr->As<ast::LiteralExpression>()) {
        return EvaluateConstantValue(e, type);
    }
    if (auto* e = expr->As<ast::CallExpression>()) {
        return EvaluateConstantValue(e, type);
    }
    return sem::Constant{};
}

utils::Result<sem::Constant> Resolver::EvaluateConstantValue(const ast::LiteralExpression* literal,
                                                             const sem::Type* type) {
    return Switch(
        literal,
        [&](const ast::BoolLiteralExpression* lit) {
            return sem::Constant{type, {AInt(lit->value ? 1 : 0)}};
        },
        [&](const ast::IntLiteralExpression* lit) {
            return sem::Constant{type, {AInt(lit->value)}};
        },
        [&](const ast::FloatLiteralExpression* lit) {
            return sem::Constant{type, {AFloat(lit->value)}};
        });
}

utils::Result<sem::Constant> Resolver::EvaluateConstantValue(const ast::CallExpression* call,
                                                             const sem::Type* ty) {
    uint32_t result_size = 0;
    auto* el_ty = sem::Type::ElementOf(ty, &result_size);
    if (!el_ty) {
        return sem::Constant{};
    }

    // ElementOf() will also return the element type of array, which we do not support.
    if (ty->Is<sem::Array>()) {
        return sem::Constant{};
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
            return sem::Constant{};
        }
        auto value = arg->ConstantValue();
        if (!value) {
            return sem::Constant{};
        }

        // Convert the elements to the desired type.
        auto converted = ConvertElements(value.GetElements(), el_ty);

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

utils::Result<sem::Constant> Resolver::ConvertValue(const sem::Constant& value,
                                                    const sem::Type* ty,
                                                    const Source& source) {
    if (value.Type() == ty) {
        return value;
    }

    auto* el_ty = sem::Type::ElementOf(ty);
    if (el_ty == nullptr) {
        return sem::Constant{};
    }
    if (value.ElementType() == el_ty) {
        return sem::Constant(ty, value.GetElements());
    }

    if (auto res = MaterializeElements(value.GetElements(), el_ty, *builder_, source)) {
        return sem::Constant(ty, std::move(res.Get()));
    }
    return utils::Failure;
}

}  // namespace tint::resolver
