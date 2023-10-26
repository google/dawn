// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/wgsl/resolver/sem_helper.h"

#include "src/tint/lang/wgsl/resolver/incomplete_type.h"
#include "src/tint/lang/wgsl/resolver/unresolved_identifier.h"
#include "src/tint/lang/wgsl/sem/builtin_enum_expression.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/function_expression.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::resolver {

SemHelper::SemHelper(ProgramBuilder* builder) : builder_(builder) {}

SemHelper::~SemHelper() = default;

std::string SemHelper::TypeNameOf(const core::type::Type* ty) const {
    return RawTypeNameOf(ty->UnwrapRef());
}

std::string SemHelper::RawTypeNameOf(const core::type::Type* ty) const {
    return ty->FriendlyName();
}

core::type::Type* SemHelper::TypeOf(const ast::Expression* expr) const {
    auto* sem = GetVal(expr);
    return sem ? const_cast<core::type::Type*>(sem->Type()) : nullptr;
}

sem::TypeExpression* SemHelper::AsTypeExpression(sem::Expression* expr) const {
    if (TINT_UNLIKELY(!expr)) {
        return nullptr;
    }

    auto* ty_expr = expr->As<sem::TypeExpression>();
    if (TINT_UNLIKELY(!ty_expr)) {
        ErrorUnexpectedExprKind(expr, "type");
        return nullptr;
    }

    auto* type = ty_expr->Type();
    if (auto* incomplete = type->As<IncompleteType>(); TINT_UNLIKELY(incomplete)) {
        AddError("expected '<' for '" + std::string(ToString(incomplete->builtin)) + "'",
                 expr->Declaration()->source.End());
        return nullptr;
    }

    return ty_expr;
}

std::string SemHelper::Describe(const sem::Expression* expr) const {
    return Switch(
        expr,  //
        [&](const sem::VariableUser* var_expr) {
            auto* variable = var_expr->Variable()->Declaration();
            auto name = variable->name->symbol.Name();
            auto* kind = Switch(
                variable,                                            //
                [&](const ast::Var*) { return "var"; },              //
                [&](const ast::Let*) { return "let"; },              //
                [&](const ast::Const*) { return "const"; },          //
                [&](const ast::Parameter*) { return "parameter"; },  //
                [&](const ast::Override*) { return "override"; },    //
                [&](Default) { return "variable"; });
            return std::string(kind) + " '" + name + "'";
        },
        [&](const sem::ValueExpression* val_expr) {
            auto type = val_expr->Type()->FriendlyName();
            return "value expression of type '" + type + "'";
        },
        [&](const sem::TypeExpression* ty_expr) {
            auto name = ty_expr->Type()->FriendlyName();
            return "type '" + name + "'";
        },
        [&](const sem::FunctionExpression* fn_expr) {
            auto* fn = fn_expr->Function()->Declaration();
            auto name = fn->name->symbol.Name();
            return "function '" + name + "'";
        },
        [&](const sem::BuiltinEnumExpression<wgsl::BuiltinFn>* fn) {
            return "builtin function '" + tint::ToString(fn->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<core::Access>* access) {
            return "access '" + tint::ToString(access->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<core::AddressSpace>* addr) {
            return "address space '" + tint::ToString(addr->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<core::BuiltinValue>* builtin) {
            return "builtin value '" + tint::ToString(builtin->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<core::InterpolationSampling>* fmt) {
            return "interpolation sampling '" + tint::ToString(fmt->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<core::InterpolationType>* fmt) {
            return "interpolation type '" + tint::ToString(fmt->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<core::TexelFormat>* fmt) {
            return "texel format '" + tint::ToString(fmt->Value()) + "'";
        },
        [&](const UnresolvedIdentifier* ui) {
            auto name = ui->Identifier()->identifier->symbol.Name();
            return "unresolved identifier '" + name + "'";
        },  //
        TINT_ICE_ON_NO_MATCH);
}

void SemHelper::ErrorUnexpectedExprKind(
    const sem::Expression* expr,
    std::string_view wanted,
    tint::Slice<char const* const> suggestions /* = Empty */) const {
    if (auto* ui = expr->As<UnresolvedIdentifier>()) {
        auto* ident = ui->Identifier();
        auto name = ident->identifier->symbol.Name();
        AddError("unresolved " + std::string(wanted) + " '" + name + "'", ident->source);
        if (!suggestions.IsEmpty()) {
            // Filter out suggestions that have a leading underscore.
            Vector<const char*, 8> filtered;
            for (auto* str : suggestions) {
                if (str[0] != '_') {
                    filtered.Push(str);
                }
            }
            StringStream msg;
            tint::SuggestAlternatives(name, filtered.Slice().Reinterpret<char const* const>(), msg);
            AddNote(msg.str(), ident->source);
        }
        return;
    }

    AddError("cannot use " + Describe(expr) + " as " + std::string(wanted),
             expr->Declaration()->source);
    NoteDeclarationSource(expr->Declaration());
}

void SemHelper::ErrorExpectedValueExpr(const sem::Expression* expr) const {
    ErrorUnexpectedExprKind(expr, "value");
    if (auto* ident = expr->Declaration()->As<ast::IdentifierExpression>()) {
        if (expr->IsAnyOf<sem::FunctionExpression, sem::TypeExpression,
                          sem::BuiltinEnumExpression<wgsl::BuiltinFn>>()) {
            AddNote("are you missing '()'?", ident->source.End());
        }
    }
}

void SemHelper::NoteDeclarationSource(const ast::Node* node) const {
    if (!node) {
        return;
    }

    Switch(
        Get(node),  //
        [&](const sem::VariableUser* var_expr) { node = var_expr->Variable()->Declaration(); },
        [&](const sem::TypeExpression* ty_expr) {
            Switch(ty_expr->Type(),  //
                   [&](const sem::Struct* s) { node = s->Declaration(); });
        },
        [&](const sem::FunctionExpression* fn_expr) { node = fn_expr->Function()->Declaration(); });

    Switch(
        node,
        [&](const ast::Struct* n) {
            AddNote("struct '" + n->name->symbol.Name() + "' declared here", n->source);
        },
        [&](const ast::Alias* n) {
            AddNote("alias '" + n->name->symbol.Name() + "' declared here", n->source);
        },
        [&](const ast::Var* n) {
            AddNote("var '" + n->name->symbol.Name() + "' declared here", n->source);
        },
        [&](const ast::Let* n) {
            AddNote("let '" + n->name->symbol.Name() + "' declared here", n->source);
        },
        [&](const ast::Override* n) {
            AddNote("override '" + n->name->symbol.Name() + "' declared here", n->source);
        },
        [&](const ast::Const* n) {
            AddNote("const '" + n->name->symbol.Name() + "' declared here", n->source);
        },
        [&](const ast::Parameter* n) {
            AddNote("parameter '" + n->name->symbol.Name() + "' declared here", n->source);
        },
        [&](const ast::Function* n) {
            AddNote("function '" + n->name->symbol.Name() + "' declared here", n->source);
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
