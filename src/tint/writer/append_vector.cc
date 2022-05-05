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
#include "src/tint/sem/expression.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/sem/type_conversion.h"
#include "src/tint/utils/transform.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer {
namespace {

struct VectorConstructorInfo {
    const sem::Call* call = nullptr;
    const sem::TypeConstructor* ctor = nullptr;
    operator bool() const { return call != nullptr; }
};
VectorConstructorInfo AsVectorConstructor(const sem::Expression* expr) {
    if (auto* call = expr->As<sem::Call>()) {
        if (auto* ctor = call->Target()->As<sem::TypeConstructor>()) {
            if (ctor->ReturnType()->Is<sem::Vector>()) {
                return {call, ctor};
            }
        }
    }
    return {};
}

const sem::Expression* Zero(ProgramBuilder& b, const sem::Type* ty, const sem::Statement* stmt) {
    const ast::Expression* expr = nullptr;
    if (ty->Is<sem::I32>()) {
        expr = b.Expr(0_i);
    } else if (ty->Is<sem::U32>()) {
        expr = b.Expr(0_u);
    } else if (ty->Is<sem::F32>()) {
        expr = b.Expr(0.0f);
    } else if (ty->Is<sem::Bool>()) {
        expr = b.Expr(false);
    } else {
        TINT_UNREACHABLE(Writer, b.Diagnostics())
            << "unsupported vector element type: " << ty->TypeInfo().name;
        return nullptr;
    }
    auto* sem = b.create<sem::Expression>(expr, ty, stmt, sem::Constant{},
                                          /* has_side_effects */ false);
    b.Sem().Add(expr, sem);
    return sem;
}

}  // namespace

const sem::Call* AppendVector(ProgramBuilder* b,
                              const ast::Expression* vector_ast,
                              const ast::Expression* scalar_ast) {
    uint32_t packed_size;
    const sem::Type* packed_el_sem_ty;
    auto* vector_sem = b->Sem().Get(vector_ast);
    auto* scalar_sem = b->Sem().Get(scalar_ast);
    auto* vector_ty = vector_sem->Type()->UnwrapRef();
    if (auto* vec = vector_ty->As<sem::Vector>()) {
        packed_size = vec->Width() + 1;
        packed_el_sem_ty = vec->type();
    } else {
        packed_size = 2;
        packed_el_sem_ty = vector_ty;
    }

    const ast::Type* packed_el_ast_ty = nullptr;
    if (packed_el_sem_ty->Is<sem::I32>()) {
        packed_el_ast_ty = b->create<ast::I32>();
    } else if (packed_el_sem_ty->Is<sem::U32>()) {
        packed_el_ast_ty = b->create<ast::U32>();
    } else if (packed_el_sem_ty->Is<sem::F32>()) {
        packed_el_ast_ty = b->create<ast::F32>();
    } else if (packed_el_sem_ty->Is<sem::Bool>()) {
        packed_el_ast_ty = b->create<ast::Bool>();
    } else {
        TINT_UNREACHABLE(Writer, b->Diagnostics())
            << "unsupported vector element type: " << packed_el_sem_ty->TypeInfo().name;
    }

    auto* statement = vector_sem->Stmt();

    auto* packed_ast_ty = b->create<ast::Vector>(packed_el_ast_ty, packed_size);
    auto* packed_sem_ty = b->create<sem::Vector>(packed_el_sem_ty, packed_size);

    // If the coordinates are already passed in a vector constructor, with only
    // scalar components supplied, extract the elements into the new vector
    // instead of nesting a vector-in-vector.
    // If the coordinates are a zero-constructor of the vector, then expand that
    // to scalar zeros.
    // The other cases for a nested vector constructor are when it is used
    // to convert a vector of a different type, e.g. vec2<i32>(vec2<u32>()).
    // In that case, preserve the original argument, or you'll get a type error.

    std::vector<const sem::Expression*> packed;
    if (auto vc = AsVectorConstructor(vector_sem)) {
        const auto num_supplied = vc.call->Arguments().size();
        if (num_supplied == 0) {
            // Zero-value vector constructor. Populate with zeros
            for (uint32_t i = 0; i < packed_size - 1; i++) {
                auto* zero = Zero(*b, packed_el_sem_ty, statement);
                packed.emplace_back(zero);
            }
        } else if (num_supplied + 1 == packed_size) {
            // All vector components were supplied as scalars.  Pass them through.
            packed = vc.call->Arguments();
        }
    }
    if (packed.empty()) {
        // The special cases didn't occur. Use the vector argument as-is.
        packed.emplace_back(vector_sem);
    }

    if (packed_el_sem_ty != scalar_sem->Type()->UnwrapRef()) {
        // Cast scalar to the vector element type
        auto* scalar_cast_ast = b->Construct(packed_el_ast_ty, scalar_ast);
        auto* scalar_cast_target = b->create<sem::TypeConversion>(
            packed_el_sem_ty,
            b->create<sem::Parameter>(nullptr, 0, scalar_sem->Type()->UnwrapRef(),
                                      ast::StorageClass::kNone, ast::Access::kUndefined));
        auto* scalar_cast_sem = b->create<sem::Call>(
            scalar_cast_ast, scalar_cast_target, std::vector<const sem::Expression*>{scalar_sem},
            statement, sem::Constant{}, /* has_side_effects */ false);
        b->Sem().Add(scalar_cast_ast, scalar_cast_sem);
        packed.emplace_back(scalar_cast_sem);
    } else {
        packed.emplace_back(scalar_sem);
    }

    auto* constructor_ast = b->Construct(
        packed_ast_ty,
        utils::Transform(packed, [&](const sem::Expression* expr) { return expr->Declaration(); }));
    auto* constructor_target = b->create<sem::TypeConstructor>(
        packed_sem_ty,
        utils::Transform(
            packed, [&](const tint::sem::Expression* arg, size_t i) -> const sem::Parameter* {
                return b->create<sem::Parameter>(nullptr, static_cast<uint32_t>(i),
                                                 arg->Type()->UnwrapRef(), ast::StorageClass::kNone,
                                                 ast::Access::kUndefined);
            }));
    auto* constructor_sem = b->create<sem::Call>(constructor_ast, constructor_target, packed,
                                                 statement, sem::Constant{},
                                                 /* has_side_effects */ false);
    b->Sem().Add(constructor_ast, constructor_sem);
    return constructor_sem;
}

}  // namespace tint::writer
