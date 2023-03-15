// Copyright 2023 The Tint Authors.
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

#include "src/tint/writer/syntax_tree/generator_impl.h"

#include <algorithm>

#include "src/tint/ast/alias.h"
#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/stride_attribute.h"
#include "src/tint/ast/struct_member_align_attribute.h"
#include "src/tint/ast/struct_member_offset_attribute.h"
#include "src/tint/ast/struct_member_size_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/switch.h"
#include "src/tint/utils/math.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/writer/float_to_string.h"

namespace tint::writer::syntax_tree {

GeneratorImpl::GeneratorImpl(const Program* program) : TextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
    // Generate global declarations in the order they appear in the module.
    for (auto* decl : program_->AST().GlobalDeclarations()) {
        if (!Switch(
                decl,  //
                [&](const ast::DiagnosticDirective* dd) {
                    return EmitDiagnosticControl(dd->control);
                },
                [&](const ast::Enable* e) { return EmitEnable(e); },
                [&](const ast::TypeDecl* td) { return EmitTypeDecl(td); },
                [&](const ast::Function* func) { return EmitFunction(func); },
                [&](const ast::Variable* var) { return EmitVariable(var); },
                [&](const ast::ConstAssert* ca) { return EmitConstAssert(ca); },
                [&](Default) {
                    TINT_UNREACHABLE(Writer, diagnostics_);
                    return false;
                })) {
            return false;
        }
        if (decl != program_->AST().GlobalDeclarations().Back()) {
            line();
        }
    }

    return true;
}

bool GeneratorImpl::EmitDiagnosticControl(const ast::DiagnosticControl& diagnostic) {
    line() << "DiagnosticControl [severity: " << diagnostic.severity
           << ", rule: " << program_->Symbols().NameFor(diagnostic.rule_name->symbol) << "]";
    return true;
}

bool GeneratorImpl::EmitEnable(const ast::Enable* enable) {
    auto l = line();
    l << "Enable [";
    for (auto* ext : enable->extensions) {
        if (ext != enable->extensions.Front()) {
            l << ", ";
        }
        l << ext->name;
    }
    l << "]";
    return true;
}

bool GeneratorImpl::EmitTypeDecl(const ast::TypeDecl* ty) {
    return Switch(
        ty,
        [&](const ast::Alias* alias) {  //
            line() << "Alias [";
            {
                ScopedIndent ai(this);

                line() << "name: " << program_->Symbols().NameFor(alias->name->symbol);
                line() << "expr: ";
                {
                    ScopedIndent ex(this);
                    if (!EmitExpression(alias->type)) {
                        return false;
                    }
                }
            }
            line() << "]";
            return true;
        },
        [&](const ast::Struct* str) {  //
            return EmitStructType(str);
        },
        [&](Default) {  //
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown declared type: " + std::string(ty->TypeInfo().name));
            return false;
        });
}

bool GeneratorImpl::EmitExpression(const ast::Expression* expr) {
    return Switch(
        expr,
        [&](const ast::IndexAccessorExpression* a) {  //
            return EmitIndexAccessor(a);
        },
        [&](const ast::BinaryExpression* b) {  //
            return EmitBinary(b);
        },
        [&](const ast::BitcastExpression* b) {  //
            return EmitBitcast(b);
        },
        [&](const ast::CallExpression* c) {  //
            return EmitCall(c);
        },
        [&](const ast::IdentifierExpression* i) {  //
            return EmitIdentifier(i);
        },
        [&](const ast::LiteralExpression* l) {  //
            return EmitLiteral(l);
        },
        [&](const ast::MemberAccessorExpression* m) {  //
            return EmitMemberAccessor(m);
        },
        [&](const ast::PhonyExpression*) {  //
            line() << "[PhonyExpression]";
            return true;
        },
        [&](const ast::UnaryOpExpression* u) {  //
            return EmitUnaryOp(u);
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer, "unknown expression type");
            return false;
        });
}

bool GeneratorImpl::EmitIndexAccessor(const ast::IndexAccessorExpression* expr) {
    line() << "IndexAccessorExpression [";
    {
        ScopedIndent iae(this);
        line() << "object: ";
        {
            ScopedIndent obj(this);
            if (!EmitExpression(expr->object)) {
                return false;
            }
        }

        line() << "index: ";
        {
            ScopedIndent idx(this);
            if (!EmitExpression(expr->index)) {
                return false;
            }
        }
    }
    line() << "]";

    return true;
}

bool GeneratorImpl::EmitMemberAccessor(const ast::MemberAccessorExpression* expr) {
    line() << "MemberAccessorExpression [";
    {
        ScopedIndent mae(this);

        line() << "object: ";
        {
            ScopedIndent obj(this);
            if (!EmitExpression(expr->object)) {
                return false;
            }
        }
        line() << "member: " << program_->Symbols().NameFor(expr->member->symbol);
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitBitcast(const ast::BitcastExpression* expr) {
    line() << "BitcastExpression [";
    {
        ScopedIndent bc(this);
        {
            line() << "type: ";
            ScopedIndent ty(this);
            if (!EmitExpression(expr->type)) {
                return false;
            }
        }
        {
            line() << "expr: ";
            ScopedIndent exp(this);
            if (!EmitExpression(expr->expr)) {
                return false;
            }
        }
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitCall(const ast::CallExpression* expr) {
    line() << "Call [";
    {
        ScopedIndent cl(this);

        line() << "target: [";
        {
            ScopedIndent tgt(this);
            if (!EmitExpression(expr->target)) {
                return false;
            }
        }
        line() << "]";

        if (!expr->args.IsEmpty()) {
            line() << "args: [";
            {
                ScopedIndent args(this);
                for (auto* arg : expr->args) {
                    line() << "arg: [";
                    {
                        ScopedIndent arg_val(this);
                        if (!EmitExpression(arg)) {
                            return false;
                        }
                    }
                    line() << "]";
                }
            }
            line() << "]";
        }
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitLiteral(const ast::LiteralExpression* lit) {
    bool ret = false;
    line() << "LiteralExpression [";
    {
        ScopedIndent le(this);
        ret = Switch(
            lit,
            [&](const ast::BoolLiteralExpression* l) {  //
                line() << (l->value ? "true" : "false");
                return true;
            },
            [&](const ast::FloatLiteralExpression* l) {  //
                // f16 literals are also emitted as float value with suffix "h".
                // Note that all normal and subnormal f16 values are normal f32 values, and since
                // NaN and Inf are not allowed to be spelled in literal, it should be fine to emit
                // f16 literals in this way.
                if (l->suffix == ast::FloatLiteralExpression::Suffix::kNone) {
                    line() << DoubleToBitPreservingString(l->value);
                } else {
                    line() << FloatToBitPreservingString(static_cast<float>(l->value)) << l->suffix;
                }
                return true;
            },
            [&](const ast::IntLiteralExpression* l) {  //
                line() << l->value << l->suffix;
                return true;
            },
            [&](Default) {  //
                diagnostics_.add_error(diag::System::Writer, "unknown literal type");
                return false;
            });
    }
    line() << "]";
    return ret;
}

bool GeneratorImpl::EmitIdentifier(const ast::IdentifierExpression* expr) {
    bool ret = false;
    line() << "IdentifierExpression [";
    {
        ScopedIndent ie(this);
        ret = EmitIdentifier(expr->identifier);
    }
    line() << "]";
    return ret;
}

bool GeneratorImpl::EmitIdentifier(const ast::Identifier* ident) {
    line() << "Identifier [";
    {
        ScopedIndent id(this);
        if (auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>()) {
            line() << "Templated [";
            {
                ScopedIndent tmpl(this);
                if (!tmpl_ident->attributes.IsEmpty()) {
                    line() << "attrs: [";
                    {
                        ScopedIndent attrs(this);
                        EmitAttributes(tmpl_ident->attributes);
                    }
                    line() << "]";
                }
                line() << "name: " << program_->Symbols().NameFor(ident->symbol);
                if (!tmpl_ident->arguments.IsEmpty()) {
                    line() << "args: [";
                    {
                        ScopedIndent args(this);
                        for (auto* expr : tmpl_ident->arguments) {
                            if (!EmitExpression(expr)) {
                                return false;
                            }
                        }
                    }
                    line() << "]";
                }
            }
            line() << "]";
        } else {
            line() << program_->Symbols().NameFor(ident->symbol);
        }
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitFunction(const ast::Function* func) {
    line() << "Function [";
    {
        ScopedIndent funct(this);

        if (func->attributes.Length()) {
            line() << "attrs: [";
            {
                ScopedIndent attrs(this);
                if (!EmitAttributes(func->attributes)) {
                    return false;
                }
            }
            line() << "]";
        }
        line() << "name: " << program_->Symbols().NameFor(func->name->symbol);

        if (!func->params.IsEmpty()) {
            line() << "params: [";
            {
                ScopedIndent args(this);
                for (auto* v : func->params) {
                    line() << "param: [";
                    {
                        ScopedIndent param(this);
                        line() << "name: " << program_->Symbols().NameFor(v->name->symbol);
                        if (!v->attributes.IsEmpty()) {
                            line() << "attrs: [";
                            {
                                ScopedIndent attrs(this);
                                if (!EmitAttributes(v->attributes)) {
                                    return false;
                                }
                            }
                            line() << "]";
                        }
                        line() << "type: [";
                        {
                            ScopedIndent ty(this);
                            if (!EmitExpression(v->type)) {
                                return false;
                            }
                        }
                        line() << "]";
                    }
                    line() << "]";
                }
            }
            line() << "]";
        }

        line() << "return: [";
        {
            ScopedIndent ret(this);

            if (func->return_type || !func->return_type_attributes.IsEmpty()) {
                if (!func->return_type_attributes.IsEmpty()) {
                    line() << "attrs: [";
                    {
                        ScopedIndent attrs(this);
                        if (!EmitAttributes(func->return_type_attributes)) {
                            return false;
                        }
                    }
                    line() << "]";
                }

                line() << "type: [";
                {
                    ScopedIndent ty(this);
                    if (!EmitExpression(func->return_type)) {
                        return false;
                    }
                }
                line() << "]";
            } else {
                line() << "void";
            }
        }
        line() << "]";
        line() << "body: [";
        {
            ScopedIndent bdy(this);
            if (func->body) {
                if (!EmitBlockHeader(func->body)) {
                    return false;
                }
                if (!EmitStatementsWithIndent(func->body->statements)) {
                    return false;
                }
            }
        }
        line() << "]";
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitImageFormat(const builtin::TexelFormat fmt) {
    line() << "builtin::TexelFormat [" << fmt << "]";
    return true;
}

bool GeneratorImpl::EmitStructType(const ast::Struct* str) {
    line() << "Struct [";
    {
        ScopedIndent strct(this);

        if (str->attributes.Length()) {
            line() << "attrs: [";
            {
                ScopedIndent attrs(this);
                if (!EmitAttributes(str->attributes)) {
                    return false;
                }
            }
            line() << "]";
        }
        line() << "name: " << program_->Symbols().NameFor(str->name->symbol);
        line() << "members: [";
        {
            ScopedIndent membs(this);

            for (auto* mem : str->members) {
                line() << "StructMember[";
                {
                    ScopedIndent m(this);
                    if (!mem->attributes.IsEmpty()) {
                        line() << "attrs: [";
                        {
                            ScopedIndent attrs(this);
                            if (!EmitAttributes(mem->attributes)) {
                                return false;
                            }
                        }
                        line() << "]";
                    }

                    line() << "name: " << program_->Symbols().NameFor(mem->name->symbol);
                    line() << "type: [";
                    {
                        ScopedIndent ty(this);
                        if (!EmitExpression(mem->type)) {
                            return false;
                        }
                    }
                    line() << "]";
                }
            }
            line() << "]";
        }
        line() << "]";
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitVariable(const ast::Variable* v) {
    line() << "Variable [";
    {
        ScopedIndent variable(this);
        if (!v->attributes.IsEmpty()) {
            line() << "attrs: [";
            {
                ScopedIndent attr(this);
                if (!EmitAttributes(v->attributes)) {
                    return false;
                }
            }
            line() << "]";
        }

        bool ok = Switch(
            v,  //
            [&](const ast::Var* var) {
                if (var->declared_address_space || var->declared_access) {
                    line() << "Var [";
                    {
                        ScopedIndent vr(this);
                        line() << "address_space: [";
                        {
                            ScopedIndent addr(this);
                            if (!EmitExpression(var->declared_address_space)) {
                                return false;
                            }
                        }
                        line() << "]";
                        if (var->declared_access) {
                            line() << "access: [";
                            {
                                ScopedIndent acs(this);
                                if (!EmitExpression(var->declared_access)) {
                                    return false;
                                }
                            }
                            line() << "]";
                        }
                    }
                    line() << "]";
                } else {
                    line() << "Var []";
                }
                return true;
            },
            [&](const ast::Let*) {
                line() << "Let []";
                return true;
            },
            [&](const ast::Override*) {
                line() << "Override []";
                return true;
            },
            [&](const ast::Const*) {
                line() << "Const []";
                return true;
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_) << "unhandled variable type " << v->TypeInfo().name;
                return false;
            });
        if (!ok) {
            return false;
        }

        line() << "name: " << program_->Symbols().NameFor(v->name->symbol);

        if (auto ty = v->type) {
            line() << "type: [";
            {
                ScopedIndent vty(this);
                if (!EmitExpression(ty)) {
                    return false;
                }
            }
            line() << "]";
        }

        if (v->initializer != nullptr) {
            line() << "initializer: [";
            {
                ScopedIndent init(this);
                if (!EmitExpression(v->initializer)) {
                    return false;
                }
            }
            line() << "]";
        }
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitAttributes(utils::VectorRef<const ast::Attribute*> attrs) {
    for (auto* attr : attrs) {
        bool ok = Switch(
            attr,
            [&](const ast::WorkgroupAttribute* workgroup) {
                auto values = workgroup->Values();
                line() << "WorkgroupAttribute [";
                {
                    ScopedIndent wg(this);
                    for (size_t i = 0; i < 3; i++) {
                        if (values[i]) {
                            if (!EmitExpression(values[i])) {
                                return false;
                            }
                        }
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::StageAttribute* stage) {
                line() << "StageAttribute [" << stage->stage << "]";
                return true;
            },
            [&](const ast::BindingAttribute* binding) {
                line() << "BindingAttribute [";
                {
                    ScopedIndent ba(this);
                    if (!EmitExpression(binding->expr)) {
                        return false;
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::GroupAttribute* group) {
                line() << "GroupAttribute [";
                {
                    ScopedIndent ga(this);
                    if (!EmitExpression(group->expr)) {
                        return false;
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::LocationAttribute* location) {
                line() << "LocationAttribute [";
                {
                    ScopedIndent la(this);
                    if (!EmitExpression(location->expr)) {
                        return false;
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::BuiltinAttribute* builtin) {
                line() << "BuiltinAttribute [";
                {
                    ScopedIndent ba(this);
                    if (!EmitExpression(builtin->builtin)) {
                        return false;
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::DiagnosticAttribute* diagnostic) {
                return EmitDiagnosticControl(diagnostic->control);
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                line() << "InterpolateAttribute [";
                {
                    ScopedIndent ia(this);
                    line() << "type: [";
                    {
                        ScopedIndent ty(this);
                        if (!EmitExpression(interpolate->type)) {
                            return false;
                        }
                    }
                    line() << "]";
                    if (interpolate->sampling) {
                        line() << "sampling: [";
                        {
                            ScopedIndent sa(this);
                            if (!EmitExpression(interpolate->sampling)) {
                                return false;
                            }
                        }
                        line() << "]";
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::InvariantAttribute*) {
                line() << "InvariantAttribute []";
                return true;
            },
            [&](const ast::IdAttribute* override_deco) {
                line() << "IdAttribute [";
                {
                    ScopedIndent id(this);
                    if (!EmitExpression(override_deco->expr)) {
                        return false;
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::MustUseAttribute*) {
                line() << "MustUseAttribute []";
                return true;
            },
            [&](const ast::StructMemberOffsetAttribute* offset) {
                line() << "StructMemberOffsetAttribute [";
                {
                    ScopedIndent smoa(this);
                    if (!EmitExpression(offset->expr)) {
                        return false;
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::StructMemberSizeAttribute* size) {
                line() << "StructMemberSizeAttribute [";
                {
                    ScopedIndent smsa(this);
                    if (!EmitExpression(size->expr)) {
                        return false;
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::StructMemberAlignAttribute* align) {
                line() << "StructMemberAlignAttribute [";
                {
                    ScopedIndent smaa(this);
                    if (!EmitExpression(align->expr)) {
                        return false;
                    }
                }
                line() << "]";
                return true;
            },
            [&](const ast::StrideAttribute* stride) {
                line() << "StrideAttribute [" << stride->stride << "]";
                return true;
            },
            [&](const ast::InternalAttribute* internal) {
                line() << "InternalAttribute [" << internal->InternalName() << "]";
                return true;
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "Unsupported attribute '" << attr->TypeInfo().name << "'";
                return false;
            });

        if (!ok) {
            return false;
        }
    }
    return true;
}

bool GeneratorImpl::EmitBinary(const ast::BinaryExpression* expr) {
    line() << "BinaryExpression [";
    {
        ScopedIndent be(this);
        line() << "lhs: [";
        {
            ScopedIndent lhs(this);

            if (!EmitExpression(expr->lhs)) {
                return false;
            }
        }
        line() << "]";
        line() << "op: [";
        {
            ScopedIndent op(this);
            if (!EmitBinaryOp(expr->op)) {
                return false;
            }
        }
        line() << "]";
        line() << "rhs: [";
        {
            ScopedIndent rhs(this);
            if (!EmitExpression(expr->rhs)) {
                return false;
            }
        }
        line() << "]";
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitBinaryOp(const ast::BinaryOp op) {
    switch (op) {
        case ast::BinaryOp::kAnd:
            line() << "&";
            break;
        case ast::BinaryOp::kOr:
            line() << "|";
            break;
        case ast::BinaryOp::kXor:
            line() << "^";
            break;
        case ast::BinaryOp::kLogicalAnd:
            line() << "&&";
            break;
        case ast::BinaryOp::kLogicalOr:
            line() << "||";
            break;
        case ast::BinaryOp::kEqual:
            line() << "==";
            break;
        case ast::BinaryOp::kNotEqual:
            line() << "!=";
            break;
        case ast::BinaryOp::kLessThan:
            line() << "<";
            break;
        case ast::BinaryOp::kGreaterThan:
            line() << ">";
            break;
        case ast::BinaryOp::kLessThanEqual:
            line() << "<=";
            break;
        case ast::BinaryOp::kGreaterThanEqual:
            line() << ">=";
            break;
        case ast::BinaryOp::kShiftLeft:
            line() << "<<";
            break;
        case ast::BinaryOp::kShiftRight:
            line() << ">>";
            break;
        case ast::BinaryOp::kAdd:
            line() << "+";
            break;
        case ast::BinaryOp::kSubtract:
            line() << "-";
            break;
        case ast::BinaryOp::kMultiply:
            line() << "*";
            break;
        case ast::BinaryOp::kDivide:
            line() << "/";
            break;
        case ast::BinaryOp::kModulo:
            line() << "%";
            break;
        case ast::BinaryOp::kNone:
            diagnostics_.add_error(diag::System::Writer, "missing binary operation type");
            return false;
    }
    return true;
}

bool GeneratorImpl::EmitUnaryOp(const ast::UnaryOpExpression* expr) {
    line() << "UnaryOpExpression [";
    {
        ScopedIndent uoe(this);
        line() << "op: [";
        {
            ScopedIndent op(this);
            switch (expr->op) {
                case ast::UnaryOp::kAddressOf:
                    line() << "&";
                    break;
                case ast::UnaryOp::kComplement:
                    line() << "~";
                    break;
                case ast::UnaryOp::kIndirection:
                    line() << "*";
                    break;
                case ast::UnaryOp::kNot:
                    line() << "!";
                    break;
                case ast::UnaryOp::kNegation:
                    line() << "-";
                    break;
            }
        }
        line() << "]";
        line() << "expr: [";
        {
            ScopedIndent ex(this);
            if (!EmitExpression(expr->expr)) {
                return false;
            }
        }
        line() << "]";
    }
    line() << "]";

    return true;
}

bool GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
    {
        if (!EmitBlockHeader(stmt)) {
            return false;
        }
    }
    if (!EmitStatementsWithIndent(stmt->statements)) {
        return false;
    }

    return true;
}

bool GeneratorImpl::EmitBlockHeader(const ast::BlockStatement* stmt) {
    if (!stmt->attributes.IsEmpty()) {
        line() << "attrs: [";
        {
            ScopedIndent attrs(this);
            if (!EmitAttributes(stmt->attributes)) {
                return false;
            }
        }
        line() << "]";
    }
    return true;
}

bool GeneratorImpl::EmitStatement(const ast::Statement* stmt) {
    return Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { return EmitAssign(a); },
        [&](const ast::BlockStatement* b) { return EmitBlock(b); },
        [&](const ast::BreakStatement* b) { return EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { return EmitBreakIf(b); },
        [&](const ast::CallStatement* c) { return EmitCall(c->expr); },
        [&](const ast::CompoundAssignmentStatement* c) { return EmitCompoundAssign(c); },
        [&](const ast::ContinueStatement* c) { return EmitContinue(c); },
        [&](const ast::DiscardStatement* d) { return EmitDiscard(d); },
        [&](const ast::IfStatement* i) { return EmitIf(i); },
        [&](const ast::IncrementDecrementStatement* l) { return EmitIncrementDecrement(l); },
        [&](const ast::LoopStatement* l) { return EmitLoop(l); },
        [&](const ast::ForLoopStatement* l) { return EmitForLoop(l); },
        [&](const ast::WhileStatement* l) { return EmitWhile(l); },
        [&](const ast::ReturnStatement* r) { return EmitReturn(r); },
        [&](const ast::ConstAssert* c) { return EmitConstAssert(c); },
        [&](const ast::SwitchStatement* s) { return EmitSwitch(s); },
        [&](const ast::VariableDeclStatement* v) { return EmitVariable(v->variable); },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown statement type: " + std::string(stmt->TypeInfo().name));
            return false;
        });
}

bool GeneratorImpl::EmitStatements(utils::VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        if (!EmitStatement(s)) {
            return false;
        }
    }
    return true;
}

bool GeneratorImpl::EmitStatementsWithIndent(utils::VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    return EmitStatements(stmts);
}

bool GeneratorImpl::EmitAssign(const ast::AssignmentStatement* stmt) {
    line() << "AssignmentStatement [";
    {
        ScopedIndent as(this);
        line() << "lhs: [";
        {
            ScopedIndent lhs(this);
            if (!EmitExpression(stmt->lhs)) {
                return false;
            }
        }
        line() << "]";
        line() << "rhs: [";
        {
            ScopedIndent rhs(this);
            if (!EmitExpression(stmt->rhs)) {
                return false;
            }
            line() << "]";
        }
    }
    line() << "]";

    return true;
}

bool GeneratorImpl::EmitBreak(const ast::BreakStatement*) {
    line() << "BreakStatement []";
    return true;
}

bool GeneratorImpl::EmitBreakIf(const ast::BreakIfStatement* b) {
    line() << "BreakIfStatement [";
    {
        ScopedIndent bis(this);
        if (!EmitExpression(b->condition)) {
            return false;
        }
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitCase(const ast::CaseStatement* stmt) {
    line() << "CaseStatement [";
    {
        ScopedIndent cs(this);
        if (stmt->selectors.Length() == 1 && stmt->ContainsDefault()) {
            line() << "selector: default";
            if (!EmitBlockHeader(stmt->body)) {
                return false;
            }
        } else {
            line() << "selectors: [";
            {
                ScopedIndent sels(this);
                for (auto* sel : stmt->selectors) {
                    if (sel->IsDefault()) {
                        line() << "default []";
                    } else if (!EmitExpression(sel->expr)) {
                        return false;
                    }
                }
            }
            line() << "]";
            if (!EmitBlockHeader(stmt->body)) {
                return false;
            }
        }
        if (!EmitStatementsWithIndent(stmt->body->statements)) {
            return false;
        }
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt) {
    line() << "CompoundAssignmentStatement [";
    {
        ScopedIndent cas(this);
        line() << "lhs: [";
        {
            ScopedIndent lhs(this);
            if (!EmitExpression(stmt->lhs)) {
                return false;
            }
        }
        line() << "]";

        line() << "op: [";
        {
            ScopedIndent op(this);
            if (!EmitBinaryOp(stmt->op)) {
                return false;
            }
        }
        line() << "]";
        line() << "rhs: [";
        {
            ScopedIndent rhs(this);

            if (!EmitExpression(stmt->rhs)) {
                return false;
            }
        }
        line() << "]";
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitContinue(const ast::ContinueStatement*) {
    line() << "ContinueStatement []";
    return true;
}

bool GeneratorImpl::EmitIf(const ast::IfStatement* stmt) {
    {
        line() << "IfStatement [";
        {
            ScopedIndent ifs(this);
            line() << "condition: [";
            {
                ScopedIndent cond(this);
                if (!EmitExpression(stmt->condition)) {
                    return false;
                }
            }
            line() << "]";
            if (!EmitBlockHeader(stmt->body)) {
                return false;
            }
        }
        line() << "] ";
    }
    if (!EmitStatementsWithIndent(stmt->body->statements)) {
        return false;
    }

    const ast::Statement* e = stmt->else_statement;
    while (e) {
        if (auto* elseif = e->As<ast::IfStatement>()) {
            {
                line() << "Else IfStatement [";
                {
                    ScopedIndent ifs(this);
                    line() << "condition: [";
                    if (!EmitExpression(elseif->condition)) {
                        return false;
                    }
                }
                line() << "]";
                if (!EmitBlockHeader(elseif->body)) {
                    return false;
                }
            }
            line() << "]";
            if (!EmitStatementsWithIndent(elseif->body->statements)) {
                return false;
            }
            e = elseif->else_statement;
        } else {
            auto* body = e->As<ast::BlockStatement>();
            {
                line() << "Else [";
                {
                    ScopedIndent els(this);
                    if (!EmitBlockHeader(body)) {
                        return false;
                    }
                }
                line() << "]";
            }
            if (!EmitStatementsWithIndent(body->statements)) {
                return false;
            }
            break;
        }
    }
    return true;
}

bool GeneratorImpl::EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt) {
    line() << "IncrementDecrementStatement [";
    {
        ScopedIndent ids(this);
        line() << "expr: [";
        if (!EmitExpression(stmt->lhs)) {
            return false;
        }
        line() << "]";
        line() << "dir: " << (stmt->increment ? "++" : "--");
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitDiscard(const ast::DiscardStatement*) {
    line() << "DiscardStatement []";
    return true;
}

bool GeneratorImpl::EmitLoop(const ast::LoopStatement* stmt) {
    line() << "LoopStatement [";
    {
        ScopedIndent ls(this);
        if (!EmitStatements(stmt->body->statements)) {
            return false;
        }

        if (stmt->continuing && !stmt->continuing->Empty()) {
            line() << "Continuing [";
            {
                ScopedIndent cont(this);
                if (!EmitStatementsWithIndent(stmt->continuing->statements)) {
                    return false;
                }
            }
            line() << "]";
        }
    }
    line() << "]";

    return true;
}

bool GeneratorImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
    TextBuffer init_buf;
    if (auto* init = stmt->initializer) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
        if (!EmitStatement(init)) {
            return false;
        }
    }

    TextBuffer cont_buf;
    if (auto* cont = stmt->continuing) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
        if (!EmitStatement(cont)) {
            return false;
        }
    }

    line() << "ForLoopStatement [";
    {
        ScopedIndent fs(this);

        line() << "initializer: [";
        {
            ScopedIndent init(this);
            switch (init_buf.lines.size()) {
                case 0:  // No initializer
                    break;
                case 1:  // Single line initializer statement
                    line() << TrimSuffix(init_buf.lines[0].content, ";");
                    break;
                default:  // Block initializer statement
                    for (size_t i = 1; i < init_buf.lines.size(); i++) {
                        // Indent all by the first line
                        init_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    line() << TrimSuffix(init_buf.String(), "\n");
                    break;
            }
        }
        line() << "]";
        line() << "condition: [";
        {
            ScopedIndent con(this);
            if (auto* cond = stmt->condition) {
                if (!EmitExpression(cond)) {
                    return false;
                }
            }
        }

        line() << "]";
        line() << "continuing: [";
        {
            ScopedIndent cont(this);
            switch (cont_buf.lines.size()) {
                case 0:  // No continuing
                    break;
                case 1:  // Single line continuing statement
                    line() << TrimSuffix(cont_buf.lines[0].content, ";");
                    break;
                default:  // Block continuing statement
                    for (size_t i = 1; i < cont_buf.lines.size(); i++) {
                        // Indent all by the first line
                        cont_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    line() << TrimSuffix(cont_buf.String(), "\n");
                    break;
            }
        }
        if (!EmitBlockHeader(stmt->body)) {
            return false;
        }

        if (!EmitStatementsWithIndent(stmt->body->statements)) {
            return false;
        }
    }
    line() << "]";

    return true;
}

bool GeneratorImpl::EmitWhile(const ast::WhileStatement* stmt) {
    line() << "WhileStatement [";
    {
        ScopedIndent ws(this);
        {
            auto* cond = stmt->condition;
            if (!EmitExpression(cond)) {
                return false;
            }
        }
        if (!EmitBlockHeader(stmt->body)) {
            return false;
        }
        if (!EmitStatementsWithIndent(stmt->body->statements)) {
            return false;
        }
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitReturn(const ast::ReturnStatement* stmt) {
    line() << "ReturnStatement [";
    {
        ScopedIndent ret(this);
        if (stmt->value) {
            if (!EmitExpression(stmt->value)) {
                return false;
            }
        }
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitConstAssert(const ast::ConstAssert* stmt) {
    line() << "ConstAssert [";
    {
        ScopedIndent ca(this);
        if (!EmitExpression(stmt->condition)) {
            return false;
        }
    }
    line() << "]";
    return true;
}

bool GeneratorImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
    line() << "SwitchStatement [";
    {
        ScopedIndent ss(this);
        line() << "condition: [";
        {
            ScopedIndent cond(this);
            if (!EmitExpression(stmt->condition)) {
                return false;
            }
        }
        line() << "]";

        {
            ScopedIndent si(this);
            for (auto* s : stmt->body) {
                if (!EmitCase(s)) {
                    return false;
                }
            }
        }
    }
    line() << "]";
    return true;
}

}  // namespace tint::writer::syntax_tree
