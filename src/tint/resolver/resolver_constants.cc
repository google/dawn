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

#include "src/tint/sem/abstract_float.h"
#include "src/tint/sem/abstract_int.h"
#include "src/tint/sem/constant.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/utils/map.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

sem::Constant::Scalars CastScalars(sem::Constant::Scalars in, const sem::Type* target_type) {
    sem::Constant::Scalars out;
    out.reserve(in.size());
    for (auto v : in) {
        // TODO(crbug.com/tint/1504): Check that value fits in new type
        out.emplace_back(Switch<sem::Constant::Scalar>(
            target_type,  //
            [&](const sem::AbstractInt*) { return sem::Constant::Cast<AInt>(v); },
            [&](const sem::AbstractFloat*) { return sem::Constant::Cast<AFloat>(v); },
            [&](const sem::I32*) { return sem::Constant::Cast<AInt>(v); },
            [&](const sem::U32*) { return sem::Constant::Cast<AInt>(v); },
            [&](const sem::F32*) { return sem::Constant::Cast<AFloat>(v); },
            [&](const sem::F16*) { return sem::Constant::Cast<AFloat>(v); },
            [&](const sem::Bool*) { return sem::Constant::Cast<bool>(v); },
            [&](Default) {
                diag::List diags;
                TINT_UNREACHABLE(Semantic, diags)
                    << "invalid element type " << target_type->TypeInfo().name;
                return sem::Constant::Scalar(false);
            }));
    }
    return out;
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
            return sem::Constant{type, {lit->value}};
        });
}

sem::Constant Resolver::EvaluateConstantValue(const ast::CallExpression* call,
                                              const sem::Type* type) {
    uint32_t result_size = 0;
    auto* el_ty = sem::Type::ElementOf(type, &result_size);
    if (!el_ty) {
        return {};
    }

    // ElementOf() will also return the element type of array, which we do not support.
    if (type->Is<sem::Array>()) {
        return {};
    }

    // For zero value init, return 0s
    if (call->args.empty()) {
        using Scalars = sem::Constant::Scalars;
        return Switch(
            el_ty,
            [&](const sem::AbstractInt*) {
                return sem::Constant(type, Scalars(result_size, AInt(0)));
            },
            [&](const sem::AbstractFloat*) {
                return sem::Constant(type, Scalars(result_size, AFloat(0)));
            },
            [&](const sem::I32*) { return sem::Constant(type, Scalars(result_size, AInt(0))); },
            [&](const sem::U32*) { return sem::Constant(type, Scalars(result_size, AInt(0))); },
            [&](const sem::F32*) { return sem::Constant(type, Scalars(result_size, AFloat(0))); },
            [&](const sem::F16*) { return sem::Constant(type, Scalars(result_size, AFloat(0))); },
            [&](const sem::Bool*) { return sem::Constant(type, Scalars(result_size, false)); });
    }

    // Build value for type_ctor from each child value by casting to type_ctor's type.
    sem::Constant::Scalars elems;
    for (auto* expr : call->args) {
        auto* arg = builder_->Sem().Get(expr);
        if (!arg) {
            return {};
        }
        auto value = arg->ConstantValue();
        if (!value) {
            return {};
        }
        elems.insert(elems.end(), value.Elements().begin(), value.Elements().end());
    }

    // Splat single-value initializers
    if (elems.size() == 1) {
        for (uint32_t i = 0; i < result_size - 1; ++i) {
            elems.emplace_back(elems[0]);
        }
    }

    // Finally cast the elements to the desired type.
    auto cast = CastScalars(elems, el_ty);

    return sem::Constant(type, std::move(cast));
}

sem::Constant Resolver::ConstantCast(const sem::Constant& value,
                                     const sem::Type* target_type,
                                     const sem::Type* target_element_type /* = nullptr */) {
    if (value.Type() == target_type) {
        return value;
    }

    if (target_element_type == nullptr) {
        target_element_type = sem::Type::ElementOf(target_type);
    }
    if (target_element_type == nullptr) {
        return {};
    }
    if (value.ElementType() == target_element_type) {
        return sem::Constant(target_type, value.Elements());
    }

    auto elems = CastScalars(value.Elements(), target_element_type);

    return sem::Constant(target_type, elems);
}

}  // namespace tint::resolver
