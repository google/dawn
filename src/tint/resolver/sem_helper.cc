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

void SemHelper::ErrorUnexpectedExprKind(const sem::Expression* expr,
                                        std::string_view wanted) const {
    Switch(
        expr,  //
        [&](const sem::VariableUser* var_expr) {
            auto* variable = var_expr->Variable()->Declaration();
            auto name = builder_->Symbols().NameFor(variable->name->symbol);
            std::string kind = Switch(
                variable,                                            //
                [&](const ast::Var*) { return "var"; },              //
                [&](const ast::Const*) { return "const"; },          //
                [&](const ast::Parameter*) { return "parameter"; },  //
                [&](const ast::Override*) { return "override"; },    //
                [&](Default) { return "variable"; });
            AddError("cannot use " + kind + " '" + name + "' as " + std::string(wanted),
                     var_expr->Declaration()->source);
            NoteDeclarationSource(variable);
        },
        [&](const sem::ValueExpression* val_expr) {
            auto type = val_expr->Type()->FriendlyName(builder_->Symbols());
            AddError("cannot use expression of type '" + type + "' as " + std::string(wanted),
                     val_expr->Declaration()->source);
        },
        [&](const sem::TypeExpression* ty_expr) {
            auto name = ty_expr->Type()->FriendlyName(builder_->Symbols());
            AddError("cannot use type '" + name + "' as " + std::string(wanted),
                     ty_expr->Declaration()->source);
        },
        [&](const sem::FunctionExpression* fn_expr) {
            auto* fn = fn_expr->Function()->Declaration();
            auto name = builder_->Symbols().NameFor(fn->name->symbol);
            AddError("cannot use function '" + name + "' as " + std::string(wanted),
                     fn_expr->Declaration()->source);
            NoteDeclarationSource(fn);
        },
        [&](const sem::BuiltinEnumExpression<builtin::Access>* access) {
            AddError("cannot use access '" + utils::ToString(access->Value()) + "' as " +
                         std::string(wanted),
                     access->Declaration()->source);
        },
        [&](const sem::BuiltinEnumExpression<builtin::AddressSpace>* addr) {
            AddError("cannot use address space '" + utils::ToString(addr->Value()) + "' as " +
                         std::string(wanted),
                     addr->Declaration()->source);
        },
        [&](const sem::BuiltinEnumExpression<builtin::BuiltinValue>* builtin) {
            AddError("cannot use builtin value '" + utils::ToString(builtin->Value()) + "' as " +
                         std::string(wanted),
                     builtin->Declaration()->source);
        },
        [&](const sem::BuiltinEnumExpression<builtin::InterpolationSampling>* fmt) {
            AddError("cannot use interpolation sampling '" + utils::ToString(fmt->Value()) +
                         "' as " + std::string(wanted),
                     fmt->Declaration()->source);
        },
        [&](const sem::BuiltinEnumExpression<builtin::InterpolationType>* fmt) {
            AddError("cannot use interpolation type '" + utils::ToString(fmt->Value()) + "' as " +
                         std::string(wanted),
                     fmt->Declaration()->source);
        },
        [&](const sem::BuiltinEnumExpression<builtin::TexelFormat>* fmt) {
            AddError("cannot use texel format '" + utils::ToString(fmt->Value()) + "' as " +
                         std::string(wanted),
                     fmt->Declaration()->source);
        },
        [&](Default) {
            TINT_ICE(Resolver, builder_->Diagnostics())
                << "unhandled sem::Expression type: " << (expr ? expr->TypeInfo().name : "<null>");
        });
}

void SemHelper::ErrorExpectedValueExpr(const sem::Expression* expr) const {
    ErrorUnexpectedExprKind(expr, "value");
    if (auto* ty_expr = expr->As<sem::TypeExpression>()) {
        if (auto* ident = ty_expr->Declaration()->As<ast::IdentifierExpression>()) {
            AddNote("are you missing '()' for value constructor?",
                    Source{{ident->source.range.end}});
        }
        if (auto* str = ty_expr->Type()->As<type::Struct>()) {
            AddNote("struct '" + str->FriendlyName(builder_->Symbols()) + "' declared here",
                    str->Source());
        }
    }
}

void SemHelper::NoteDeclarationSource(const ast::Node* node) const {
    Switch(
        node,
        [&](const ast::Struct* n) {
            AddNote("struct '" + builder_->Symbols().NameFor(n->name->symbol) + "' declared here",
                    n->source);
        },
        [&](const ast::Alias* n) {
            AddNote("alias '" + builder_->Symbols().NameFor(n->name->symbol) + "' declared here",
                    n->source);
        },
        [&](const ast::Var* n) {
            AddNote("var '" + builder_->Symbols().NameFor(n->name->symbol) + "' declared here",
                    n->source);
        },
        [&](const ast::Let* n) {
            AddNote("let '" + builder_->Symbols().NameFor(n->name->symbol) + "' declared here",
                    n->source);
        },
        [&](const ast::Override* n) {
            AddNote("override '" + builder_->Symbols().NameFor(n->name->symbol) + "' declared here",
                    n->source);
        },
        [&](const ast::Const* n) {
            AddNote("const '" + builder_->Symbols().NameFor(n->name->symbol) + "' declared here",
                    n->source);
        },
        [&](const ast::Parameter* n) {
            AddNote(
                "parameter '" + builder_->Symbols().NameFor(n->name->symbol) + "' declared here",
                n->source);
        },
        [&](const ast::Function* n) {
            AddNote("function '" + builder_->Symbols().NameFor(n->name->symbol) + "' declared here",
                    n->source);
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
