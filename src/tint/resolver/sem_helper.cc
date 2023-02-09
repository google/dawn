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

#include "src/tint/sem/type_expression.h"
#include "src/tint/sem/value_expression.h"

namespace tint::resolver {

SemHelper::SemHelper(ProgramBuilder* builder) : builder_(builder) {}

SemHelper::~SemHelper() = default;

std::string SemHelper::TypeNameOf(const type::Type* ty) const {
    return RawTypeNameOf(ty->UnwrapRef());
}

std::string SemHelper::RawTypeNameOf(const type::Type* ty) const {
    return ty->FriendlyName(builder_->Symbols());
}

type::Type* SemHelper::TypeOf(const ast::Expression* expr) const {
    auto* sem = GetVal(expr);
    return sem ? const_cast<type::Type*>(sem->Type()) : nullptr;
}

void SemHelper::ErrorExpectedValueExpr(const sem::Expression* expr) const {
    Switch(
        expr,  //
        [&](const sem::TypeExpression* ty_expr) {
            auto name = ty_expr->Type()->FriendlyName(builder_->Symbols());
            AddError("cannot use type '" + name + "' as value", ty_expr->Declaration()->source);
            if (auto* ident = ty_expr->Declaration()->As<ast::IdentifierExpression>()) {
                AddNote("are you missing '()' for type initializer?",
                        Source{{ident->source.range.end}});
            }
            if (auto* str = ty_expr->Type()->As<type::Struct>()) {
                AddNote("'" + name + "' declared here", str->Source());
            }
        },
        [&](Default) {
            TINT_ICE(Resolver, builder_->Diagnostics())
                << "unhandled sem::Expression type: " << (expr ? expr->TypeInfo().name : "<null>");
        });
}

void SemHelper::AddError(const std::string& msg, const Source& source) const {
    builder_->Diagnostics().add_error(diag::System::Resolver, msg, source);
}

void SemHelper::AddWarning(const std::string& msg, const Source& source) const {
    builder_->Diagnostics().add_warning(diag::System::Resolver, msg, source);
}

void SemHelper::AddNote(const std::string& msg, const Source& source) const {
    builder_->Diagnostics().add_note(diag::System::Resolver, msg, source);
}
}  // namespace tint::resolver
