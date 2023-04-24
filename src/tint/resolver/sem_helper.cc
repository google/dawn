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

#include "src/tint/sem/builtin_enum_expression.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/function_expression.h"
#include "src/tint/sem/type_expression.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/switch.h"

namespace tint::resolver {

SemHelper::SemHelper(ProgramBuilder* builder) : builder_(builder) {}

SemHelper::~SemHelper() = default;

std::string SemHelper::TypeNameOf(const type::Type* ty) const {
    return RawTypeNameOf(ty->UnwrapRef());
}

std::string SemHelper::RawTypeNameOf(const type::Type* ty) const {
    return ty->FriendlyName();
}

type::Type* SemHelper::TypeOf(const ast::Expression* expr) const {
    auto* sem = GetVal(expr);
    return sem ? const_cast<type::Type*>(sem->Type()) : nullptr;
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
        [&](const sem::BuiltinEnumExpression<builtin::Access>* access) {
            return "access '" + utils::ToString(access->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<builtin::AddressSpace>* addr) {
            return "address space '" + utils::ToString(addr->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<builtin::BuiltinValue>* builtin) {
            return "builtin value '" + utils::ToString(builtin->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<builtin::InterpolationSampling>* fmt) {
            return "interpolation sampling '" + utils::ToString(fmt->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<builtin::InterpolationType>* fmt) {
            return "interpolation type '" + utils::ToString(fmt->Value()) + "'";
        },
        [&](const sem::BuiltinEnumExpression<builtin::TexelFormat>* fmt) {
            return "texel format '" + utils::ToString(fmt->Value()) + "'";
        },
        [&](Default) -> std::string {
            TINT_ICE(Resolver, builder_->Diagnostics())
                << "unhandled sem::Expression type: " << (expr ? expr->TypeInfo().name : "<null>");
            return "<unknown>";
        });
}

void SemHelper::ErrorUnexpectedExprKind(const sem::Expression* expr,
                                        std::string_view wanted) const {
    AddError("cannot use " + Describe(expr) + " as " + std::string(wanted),
             expr->Declaration()->source);
    NoteDeclarationSource(expr->Declaration());
}

void SemHelper::ErrorExpectedValueExpr(const sem::Expression* expr) const {
    ErrorUnexpectedExprKind(expr, "value");
    if (auto* ty_expr = expr->As<sem::TypeExpression>()) {
        if (auto* ident = ty_expr->Declaration()->As<ast::IdentifierExpression>()) {
            AddNote("are you missing '()' for value constructor?", ident->source.End());
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
