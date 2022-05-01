// Copyright 2022 The Tint Authors.
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

#include "src/tint/resolver/sem_helper.h"

#include "src/tint/sem/expression.h"

namespace tint::resolver {

SemHelper::SemHelper(ProgramBuilder* builder, DependencyGraph& dependencies)
    : builder_(builder), dependencies_(dependencies) {}

SemHelper::~SemHelper() = default;

std::string SemHelper::TypeNameOf(const sem::Type* ty) const {
    return RawTypeNameOf(ty->UnwrapRef());
}

std::string SemHelper::RawTypeNameOf(const sem::Type* ty) const {
    return ty->FriendlyName(builder_->Symbols());
}

sem::Type* SemHelper::TypeOf(const ast::Expression* expr) const {
    auto* sem = Get(expr);
    return sem ? const_cast<sem::Type*>(sem->Type()) : nullptr;
}

sem::Type* SemHelper::TypeOf(const ast::LiteralExpression* lit) {
    return Switch(
        lit, [&](const ast::SintLiteralExpression*) { return builder_->create<sem::I32>(); },
        [&](const ast::UintLiteralExpression*) { return builder_->create<sem::U32>(); },
        [&](const ast::FloatLiteralExpression*) { return builder_->create<sem::F32>(); },
        [&](const ast::BoolLiteralExpression*) { return builder_->create<sem::Bool>(); },
        [&](Default) {
            TINT_UNREACHABLE(Resolver, builder_->Diagnostics())
                << "Unhandled literal type: " << lit->TypeInfo().name;
            return nullptr;
        });
}

}  // namespace tint::resolver
