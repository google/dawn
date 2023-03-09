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

#include "src/tint/writer/append_vector.h"

#include <utility>
#include <vector>

#include "src/tint/sem/call.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/sem/value_conversion.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/switch.h"
#include "src/tint/utils/transform.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer {
namespace {

struct VectorConstructorInfo {
    const sem::Call* call = nullptr;
    const sem::ValueConstructor* ctor = nullptr;
    operator bool() const { return call != nullptr; }
};
VectorConstructorInfo AsVectorConstructor(const sem::ValueExpression* expr) {
    if (auto* call = expr->As<sem::Call>()) {
        if (auto* ctor = call->Target()->As<sem::ValueConstructor>()) {
            if (ctor->ReturnType()->Is<type::Vector>()) {
                return {call, ctor};
            }
        }
    }
    return {};
}

const sem::ValueExpression* Zero(ProgramBuilder& b,
                                 const type::Type* ty,
                                 const sem::Statement* stmt) {
    const ast::Expression* expr = nullptr;
    if (ty->Is<type::I32>()) {
        expr = b.Expr(0_i);
    } else if (ty->Is<type::U32>()) {
        expr = b.Expr(0_u);
    } else if (ty->Is<type::F32>()) {
        expr = b.Expr(0_f);
    } else if (ty->Is<type::Bool>()) {
        expr = b.Expr(false);
    } else {
        TINT_UNREACHABLE(Writer, b.Diagnostics())
            << "unsupported vector element type: " << ty->TypeInfo().name;
        return nullptr;
    }
    auto* sem = b.create<sem::ValueExpression>(expr, ty, sem::EvaluationStage::kRuntime, stmt,
                                               /* constant_value */ nullptr,
                                               /* has_side_effects */ false);
    b.Sem().Add(expr, sem);
    return sem;
}

}  // namespace

const sem::Call* AppendVector(ProgramBuilder* b,
                              const ast::Expression* vector_ast,
                              const ast::Expression* scalar_ast) {
    uint32_t packed_size;
    const type::Type* packed_el_sem_ty;
    auto* vector_sem = b->Sem().GetVal(vector_ast);
    auto* scalar_sem = b->Sem().GetVal(scalar_ast);
    auto* vector_ty = vector_sem->Type()->UnwrapRef();
    if (auto* vec = vector_ty->As<type::Vector>()) {
        packed_size = vec->Width() + 1;
        packed_el_sem_ty = vec->type();
    } else {
        packed_size = 2;
        packed_el_sem_ty = vector_ty;
    }

    auto packed_el_ast_ty = Switch(
        packed_el_sem_ty,  //
        [&](const type::I32*) { return b->ty.i32(); },
        [&](const type::U32*) { return b->ty.u32(); },
        [&](const type::F32*) { return b->ty.f32(); },
        [&](const type::Bool*) { return b->ty.bool_(); },
        [&](Default) {
            TINT_UNREACHABLE(Writer, b->Diagnostics())
                << "unsupported vector element type: " << packed_el_sem_ty->TypeInfo().name;
            return ast::Type{};
        });

    auto* statement = vector_sem->Stmt();

    auto packed_ast_ty = b->ty.vec(packed_el_ast_ty, packed_size);
    auto* packed_sem_ty = b->create<type::Vector>(packed_el_sem_ty, packed_size);

    // If the coordinates are already passed in a vector constructor, with only
    // scalar components supplied, extract the elements into the new vector
    // instead of nesting a vector-in-vector.
    // If the coordinates are a zero-constructor of the vector, then expand that
    // to scalar zeros.
    // The other cases for a nested vector constructor are when it is used
    // to convert a vector of a different type, e.g. vec2<i32>(vec2<u32>()).
    // In that case, preserve the original argument, or you'll get a type error.

    utils::Vector<const sem::ValueExpression*, 4> packed;
    if (auto vc = AsVectorConstructor(vector_sem)) {
        const auto num_supplied = vc.call->Arguments().Length();
        if (num_supplied == 0) {
            // Zero-value vector constructor. Populate with zeros
            for (uint32_t i = 0; i < packed_size - 1; i++) {
                auto* zero = Zero(*b, packed_el_sem_ty, statement);
                packed.Push(zero);
            }
        } else if (num_supplied + 1 == packed_size) {
            // All vector components were supplied as scalars.  Pass them through.
            packed = vc.call->Arguments();
        }
    }
    if (packed.IsEmpty()) {
        // The special cases didn't occur. Use the vector argument as-is.
        packed.Push(vector_sem);
    }

    if (packed_el_sem_ty != scalar_sem->Type()->UnwrapRef()) {
        // Cast scalar to the vector element type
        auto* scalar_cast_ast = b->Call(packed_el_ast_ty, scalar_ast);
        auto* scalar_cast_target = b->create<sem::ValueConversion>(
            packed_el_sem_ty,
            b->create<sem::Parameter>(nullptr, 0u, scalar_sem->Type()->UnwrapRef(),
                                      builtin::AddressSpace::kUndefined,
                                      builtin::Access::kUndefined),
            sem::EvaluationStage::kRuntime);
        auto* scalar_cast_sem = b->create<sem::Call>(
            scalar_cast_ast, scalar_cast_target, sem::EvaluationStage::kRuntime,
            utils::Vector<const sem::ValueExpression*, 1>{scalar_sem}, statement,
            /* constant_value */ nullptr, /* has_side_effects */ false);
        b->Sem().Add(scalar_cast_ast, scalar_cast_sem);
        packed.Push(scalar_cast_sem);
    } else {
        packed.Push(scalar_sem);
    }

    auto* ctor_ast =
        b->Call(packed_ast_ty, utils::Transform(packed, [&](const sem::ValueExpression* expr) {
                    return expr->Declaration();
                }));
    auto* ctor_target = b->create<sem::ValueConstructor>(
        packed_sem_ty,
        utils::Transform(packed,
                         [&](const tint::sem::ValueExpression* arg, size_t i) {
                             return b->create<sem::Parameter>(
                                 nullptr, static_cast<uint32_t>(i), arg->Type()->UnwrapRef(),
                                 builtin::AddressSpace::kUndefined, builtin::Access::kUndefined);
                         }),
        sem::EvaluationStage::kRuntime);
    auto* ctor_sem = b->create<sem::Call>(ctor_ast, ctor_target, sem::EvaluationStage::kRuntime,
                                          std::move(packed), statement,
                                          /* constant_value */ nullptr,
                                          /* has_side_effects */ false);
    b->Sem().Add(ctor_ast, ctor_sem);
    return ctor_sem;
}

}  // namespace tint::writer
