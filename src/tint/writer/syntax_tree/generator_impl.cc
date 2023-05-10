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

void GeneratorImpl::Generate() {
    // Generate global declarations in the order they appear in the module.
    for (auto* decl : program_->AST().GlobalDeclarations()) {
        Switch(
            decl,  //
            [&](const ast::DiagnosticDirective* dd) { EmitDiagnosticControl(dd->control); },
            [&](const ast::Enable* e) { EmitEnable(e); },
            [&](const ast::TypeDecl* td) { EmitTypeDecl(td); },
            [&](const ast::Function* func) { EmitFunction(func); },
            [&](const ast::Variable* var) { EmitVariable(var); },
            [&](const ast::ConstAssert* ca) { EmitConstAssert(ca); },
            [&](Default) { TINT_UNREACHABLE(Writer, diagnostics_); });

        if (decl != program_->AST().GlobalDeclarations().Back()) {
            line();
        }
    }
}

void GeneratorImpl::EmitDiagnosticControl(const ast::DiagnosticControl& diagnostic) {
    line() << "DiagnosticControl [severity: " << diagnostic.severity
           << ", rule: " << diagnostic.rule_name->String() << "]";
}

void GeneratorImpl::EmitEnable(const ast::Enable* enable) {
    auto l = line();
    l << "Enable [";
    for (auto* ext : enable->extensions) {
        if (ext != enable->extensions.Front()) {
            l << ", ";
        }
        l << ext->name;
    }
    l << "]";
}

void GeneratorImpl::EmitTypeDecl(const ast::TypeDecl* ty) {
    Switch(
        ty,  //
        [&](const ast::Alias* alias) {
            line() << "Alias [";
            {
                ScopedIndent ai(this);

                line() << "name: " << alias->name->symbol.Name();
                line() << "expr: ";
                {
                    ScopedIndent ex(this);
                    EmitExpression(alias->type);
                }
            }
            line() << "]";
        },
        [&](const ast::Struct* str) { EmitStructType(str); },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown declared type: " + std::string(ty->TypeInfo().name));
        });
}

void GeneratorImpl::EmitExpression(const ast::Expression* expr) {
    Switch(
        expr,  //
        [&](const ast::IndexAccessorExpression* a) { EmitIndexAccessor(a); },
        [&](const ast::BinaryExpression* b) { EmitBinary(b); },
        [&](const ast::BitcastExpression* b) { EmitBitcast(b); },
        [&](const ast::CallExpression* c) { EmitCall(c); },
        [&](const ast::IdentifierExpression* i) { EmitIdentifier(i); },
        [&](const ast::LiteralExpression* l) { EmitLiteral(l); },
        [&](const ast::MemberAccessorExpression* m) { EmitMemberAccessor(m); },
        [&](const ast::PhonyExpression*) { line() << "[PhonyExpression]"; },
        [&](const ast::UnaryOpExpression* u) { EmitUnaryOp(u); },
        [&](Default) { diagnostics_.add_error(diag::System::Writer, "unknown expression type"); });
}

void GeneratorImpl::EmitIndexAccessor(const ast::IndexAccessorExpression* expr) {
    line() << "IndexAccessorExpression [";
    {
        ScopedIndent iae(this);
        line() << "object: ";
        {
            ScopedIndent obj(this);
            EmitExpression(expr->object);
        }

        line() << "index: ";
        {
            ScopedIndent idx(this);
            EmitExpression(expr->index);
        }
    }
    line() << "]";
}

void GeneratorImpl::EmitMemberAccessor(const ast::MemberAccessorExpression* expr) {
    line() << "MemberAccessorExpression [";
    {
        ScopedIndent mae(this);

        line() << "object: ";
        {
            ScopedIndent obj(this);
            EmitExpression(expr->object);
        }
        line() << "member: " << expr->member->symbol.Name();
    }
    line() << "]";
}

void GeneratorImpl::EmitBitcast(const ast::BitcastExpression* expr) {
    line() << "BitcastExpression [";
    {
        ScopedIndent bc(this);
        {
            line() << "type: ";
            ScopedIndent ty(this);
            EmitExpression(expr->type);
        }
        {
            line() << "expr: ";
            ScopedIndent exp(this);
            EmitExpression(expr->expr);
        }
    }
    line() << "]";
}

void GeneratorImpl::EmitCall(const ast::CallExpression* expr) {
    line() << "Call [";
    {
        ScopedIndent cl(this);

        line() << "target: [";
        {
            ScopedIndent tgt(this);
            EmitExpression(expr->target);
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
                        EmitExpression(arg);
                    }
                    line() << "]";
                }
            }
            line() << "]";
        }
    }
    line() << "]";
}

void GeneratorImpl::EmitLiteral(const ast::LiteralExpression* lit) {
    line() << "LiteralExpression [";
    {
        ScopedIndent le(this);
        Switch(
            lit,  //
            [&](const ast::BoolLiteralExpression* l) { line() << (l->value ? "true" : "false"); },
            [&](const ast::FloatLiteralExpression* l) {
                // f16 literals are also emitted as float value with suffix "h".
                // Note that all normal and subnormal f16 values are normal f32 values, and since
                // NaN and Inf are not allowed to be spelled in literal, it should be fine to emit
                // f16 literals in this way.
                if (l->suffix == ast::FloatLiteralExpression::Suffix::kNone) {
                    line() << DoubleToBitPreservingString(l->value);
                } else {
                    line() << FloatToBitPreservingString(static_cast<float>(l->value)) << l->suffix;
                }
            },
            [&](const ast::IntLiteralExpression* l) { line() << l->value << l->suffix; },
            [&](Default) { diagnostics_.add_error(diag::System::Writer, "unknown literal type"); });
    }
    line() << "]";
}

void GeneratorImpl::EmitIdentifier(const ast::IdentifierExpression* expr) {
    line() << "IdentifierExpression [";
    {
        ScopedIndent ie(this);
        EmitIdentifier(expr->identifier);
    }
    line() << "]";
}

void GeneratorImpl::EmitIdentifier(const ast::Identifier* ident) {
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
                line() << "name: " << ident->symbol.Name();
                if (!tmpl_ident->arguments.IsEmpty()) {
                    line() << "args: [";
                    {
                        ScopedIndent args(this);
                        for (auto* expr : tmpl_ident->arguments) {
                            EmitExpression(expr);
                        }
                    }
                    line() << "]";
                }
            }
            line() << "]";
        } else {
            line() << ident->symbol.Name();
        }
    }
    line() << "]";
}

void GeneratorImpl::EmitFunction(const ast::Function* func) {
    line() << "Function [";
    {
        ScopedIndent funct(this);

        if (func->attributes.Length()) {
            line() << "attrs: [";
            {
                ScopedIndent attrs(this);
                EmitAttributes(func->attributes);
            }
            line() << "]";
        }
        line() << "name: " << func->name->symbol.Name();

        if (!func->params.IsEmpty()) {
            line() << "params: [";
            {
                ScopedIndent args(this);
                for (auto* v : func->params) {
                    line() << "param: [";
                    {
                        ScopedIndent param(this);
                        line() << "name: " << v->name->symbol.Name();
                        if (!v->attributes.IsEmpty()) {
                            line() << "attrs: [";
                            {
                                ScopedIndent attrs(this);
                                EmitAttributes(v->attributes);
                            }
                            line() << "]";
                        }
                        line() << "type: [";
                        {
                            ScopedIndent ty(this);
                            EmitExpression(v->type);
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
                        EmitAttributes(func->return_type_attributes);
                    }
                    line() << "]";
                }

                line() << "type: [";
                {
                    ScopedIndent ty(this);
                    EmitExpression(func->return_type);
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
                EmitBlockHeader(func->body);
                EmitStatementsWithIndent(func->body->statements);
            }
        }
        line() << "]";
    }
    line() << "]";
}

void GeneratorImpl::EmitImageFormat(const builtin::TexelFormat fmt) {
    line() << "builtin::TexelFormat [" << fmt << "]";
}

void GeneratorImpl::EmitStructType(const ast::Struct* str) {
    line() << "Struct [";
    {
        ScopedIndent strct(this);

        if (str->attributes.Length()) {
            line() << "attrs: [";
            {
                ScopedIndent attrs(this);
                EmitAttributes(str->attributes);
            }
            line() << "]";
        }
        line() << "name: " << str->name->symbol.Name();
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
                            EmitAttributes(mem->attributes);
                        }
                        line() << "]";
                    }

                    line() << "name: " << mem->name->symbol.Name();
                    line() << "type: [";
                    {
                        ScopedIndent ty(this);
                        EmitExpression(mem->type);
                    }
                    line() << "]";
                }
            }
            line() << "]";
        }
        line() << "]";
    }
    line() << "]";
}

void GeneratorImpl::EmitVariable(const ast::Variable* v) {
    line() << "Variable [";
    {
        ScopedIndent variable(this);
        if (!v->attributes.IsEmpty()) {
            line() << "attrs: [";
            {
                ScopedIndent attr(this);
                EmitAttributes(v->attributes);
            }
            line() << "]";
        }

        Switch(
            v,  //
            [&](const ast::Var* var) {
                if (var->declared_address_space || var->declared_access) {
                    line() << "Var [";
                    {
                        ScopedIndent vr(this);
                        line() << "address_space: [";
                        {
                            ScopedIndent addr(this);
                            EmitExpression(var->declared_address_space);
                        }
                        line() << "]";
                        if (var->declared_access) {
                            line() << "access: [";
                            {
                                ScopedIndent acs(this);
                                EmitExpression(var->declared_access);
                            }
                            line() << "]";
                        }
                    }
                    line() << "]";
                } else {
                    line() << "Var []";
                }
            },
            [&](const ast::Let*) { line() << "Let []"; },
            [&](const ast::Override*) { line() << "Override []"; },
            [&](const ast::Const*) { line() << "Const []"; },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_) << "unhandled variable type " << v->TypeInfo().name;
            });

        line() << "name: " << v->name->symbol.Name();

        if (auto ty = v->type) {
            line() << "type: [";
            {
                ScopedIndent vty(this);
                EmitExpression(ty);
            }
            line() << "]";
        }

        if (v->initializer != nullptr) {
            line() << "initializer: [";
            {
                ScopedIndent init(this);
                EmitExpression(v->initializer);
            }
            line() << "]";
        }
    }
    line() << "]";
}

void GeneratorImpl::EmitAttributes(utils::VectorRef<const ast::Attribute*> attrs) {
    for (auto* attr : attrs) {
        Switch(
            attr,  //
            [&](const ast::WorkgroupAttribute* workgroup) {
                auto values = workgroup->Values();
                line() << "WorkgroupAttribute [";
                {
                    ScopedIndent wg(this);
                    for (size_t i = 0; i < 3; i++) {
                        if (values[i]) {
                            EmitExpression(values[i]);
                        }
                    }
                }
                line() << "]";
            },
            [&](const ast::StageAttribute* stage) {
                line() << "StageAttribute [" << stage->stage << "]";
            },
            [&](const ast::BindingAttribute* binding) {
                line() << "BindingAttribute [";
                {
                    ScopedIndent ba(this);
                    EmitExpression(binding->expr);
                }
                line() << "]";
            },
            [&](const ast::GroupAttribute* group) {
                line() << "GroupAttribute [";
                {
                    ScopedIndent ga(this);
                    EmitExpression(group->expr);
                }
                line() << "]";
            },
            [&](const ast::LocationAttribute* location) {
                line() << "LocationAttribute [";
                {
                    ScopedIndent la(this);
                    EmitExpression(location->expr);
                }
                line() << "]";
            },
            [&](const ast::BuiltinAttribute* builtin) {
                line() << "BuiltinAttribute [";
                {
                    ScopedIndent ba(this);
                    EmitExpression(builtin->builtin);
                }
                line() << "]";
            },
            [&](const ast::DiagnosticAttribute* diagnostic) {
                EmitDiagnosticControl(diagnostic->control);
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                line() << "InterpolateAttribute [";
                {
                    ScopedIndent ia(this);
                    line() << "type: [";
                    {
                        ScopedIndent ty(this);
                        EmitExpression(interpolate->type);
                    }
                    line() << "]";
                    if (interpolate->sampling) {
                        line() << "sampling: [";
                        {
                            ScopedIndent sa(this);
                            EmitExpression(interpolate->sampling);
                        }
                        line() << "]";
                    }
                }
                line() << "]";
            },
            [&](const ast::InvariantAttribute*) { line() << "InvariantAttribute []"; },
            [&](const ast::IdAttribute* override_deco) {
                line() << "IdAttribute [";
                {
                    ScopedIndent id(this);
                    EmitExpression(override_deco->expr);
                }
                line() << "]";
            },
            [&](const ast::MustUseAttribute*) { line() << "MustUseAttribute []"; },
            [&](const ast::StructMemberOffsetAttribute* offset) {
                line() << "StructMemberOffsetAttribute [";
                {
                    ScopedIndent smoa(this);
                    EmitExpression(offset->expr);
                }
                line() << "]";
            },
            [&](const ast::StructMemberSizeAttribute* size) {
                line() << "StructMemberSizeAttribute [";
                {
                    ScopedIndent smsa(this);
                    EmitExpression(size->expr);
                }
                line() << "]";
            },
            [&](const ast::StructMemberAlignAttribute* align) {
                line() << "StructMemberAlignAttribute [";
                {
                    ScopedIndent smaa(this);
                    EmitExpression(align->expr);
                }
                line() << "]";
            },
            [&](const ast::StrideAttribute* stride) {
                line() << "StrideAttribute [" << stride->stride << "]";
            },
            [&](const ast::InternalAttribute* internal) {
                line() << "InternalAttribute [" << internal->InternalName() << "]";
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "Unsupported attribute '" << attr->TypeInfo().name << "'";
            });
    }
}

void GeneratorImpl::EmitBinary(const ast::BinaryExpression* expr) {
    line() << "BinaryExpression [";
    {
        ScopedIndent be(this);
        line() << "lhs: [";
        {
            ScopedIndent lhs(this);

            EmitExpression(expr->lhs);
        }
        line() << "]";
        line() << "op: [";
        {
            ScopedIndent op(this);
            EmitBinaryOp(expr->op);
        }
        line() << "]";
        line() << "rhs: [";
        {
            ScopedIndent rhs(this);
            EmitExpression(expr->rhs);
        }
        line() << "]";
    }
    line() << "]";
}

void GeneratorImpl::EmitBinaryOp(const ast::BinaryOp op) {
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
            break;
    }
}

void GeneratorImpl::EmitUnaryOp(const ast::UnaryOpExpression* expr) {
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
            EmitExpression(expr->expr);
        }
        line() << "]";
    }
    line() << "]";
}

void GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
    EmitBlockHeader(stmt);
    EmitStatementsWithIndent(stmt->statements);
}

void GeneratorImpl::EmitBlockHeader(const ast::BlockStatement* stmt) {
    if (!stmt->attributes.IsEmpty()) {
        line() << "attrs: [";
        {
            ScopedIndent attrs(this);
            EmitAttributes(stmt->attributes);
        }
        line() << "]";
    }
}

void GeneratorImpl::EmitStatement(const ast::Statement* stmt) {
    Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { EmitAssign(a); },
        [&](const ast::BlockStatement* b) { EmitBlock(b); },
        [&](const ast::BreakStatement* b) { EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { EmitBreakIf(b); },
        [&](const ast::CallStatement* c) { EmitCall(c->expr); },
        [&](const ast::CompoundAssignmentStatement* c) { EmitCompoundAssign(c); },
        [&](const ast::ContinueStatement* c) { EmitContinue(c); },
        [&](const ast::DiscardStatement* d) { EmitDiscard(d); },
        [&](const ast::IfStatement* i) { EmitIf(i); },
        [&](const ast::IncrementDecrementStatement* l) { EmitIncrementDecrement(l); },
        [&](const ast::LoopStatement* l) { EmitLoop(l); },
        [&](const ast::ForLoopStatement* l) { EmitForLoop(l); },
        [&](const ast::WhileStatement* l) { EmitWhile(l); },
        [&](const ast::ReturnStatement* r) { EmitReturn(r); },
        [&](const ast::ConstAssert* c) { EmitConstAssert(c); },
        [&](const ast::SwitchStatement* s) { EmitSwitch(s); },
        [&](const ast::VariableDeclStatement* v) { EmitVariable(v->variable); },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown statement type: " + std::string(stmt->TypeInfo().name));
        });
}

void GeneratorImpl::EmitStatements(utils::VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        EmitStatement(s);
    }
}

void GeneratorImpl::EmitStatementsWithIndent(utils::VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    EmitStatements(stmts);
}

void GeneratorImpl::EmitAssign(const ast::AssignmentStatement* stmt) {
    line() << "AssignmentStatement [";
    {
        ScopedIndent as(this);
        line() << "lhs: [";
        {
            ScopedIndent lhs(this);
            EmitExpression(stmt->lhs);
        }
        line() << "]";
        line() << "rhs: [";
        {
            ScopedIndent rhs(this);
            EmitExpression(stmt->rhs);
        }
        line() << "]";
    }
    line() << "]";
}

void GeneratorImpl::EmitBreak(const ast::BreakStatement*) {
    line() << "BreakStatement []";
}

void GeneratorImpl::EmitBreakIf(const ast::BreakIfStatement* b) {
    line() << "BreakIfStatement [";
    {
        ScopedIndent bis(this);
        EmitExpression(b->condition);
    }
    line() << "]";
}

void GeneratorImpl::EmitCase(const ast::CaseStatement* stmt) {
    line() << "CaseStatement [";
    {
        ScopedIndent cs(this);
        if (stmt->selectors.Length() == 1 && stmt->ContainsDefault()) {
            line() << "selector: default";
            EmitBlockHeader(stmt->body);
        } else {
            line() << "selectors: [";
            {
                ScopedIndent sels(this);
                for (auto* sel : stmt->selectors) {
                    if (sel->IsDefault()) {
                        line() << "default []";
                    } else {
                        EmitExpression(sel->expr);
                    }
                }
            }
            line() << "]";
            EmitBlockHeader(stmt->body);
        }
        EmitStatementsWithIndent(stmt->body->statements);
    }
    line() << "]";
}

void GeneratorImpl::EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt) {
    line() << "CompoundAssignmentStatement [";
    {
        ScopedIndent cas(this);
        line() << "lhs: [";
        {
            ScopedIndent lhs(this);
            EmitExpression(stmt->lhs);
        }
        line() << "]";

        line() << "op: [";
        {
            ScopedIndent op(this);
            EmitBinaryOp(stmt->op);
        }
        line() << "]";
        line() << "rhs: [";
        {
            ScopedIndent rhs(this);

            EmitExpression(stmt->rhs);
        }
        line() << "]";
    }
    line() << "]";
}

void GeneratorImpl::EmitContinue(const ast::ContinueStatement*) {
    line() << "ContinueStatement []";
}

void GeneratorImpl::EmitIf(const ast::IfStatement* stmt) {
    {
        line() << "IfStatement [";
        {
            ScopedIndent ifs(this);
            line() << "condition: [";
            {
                ScopedIndent cond(this);
                EmitExpression(stmt->condition);
            }
            line() << "]";
            EmitBlockHeader(stmt->body);
        }
        line() << "] ";
    }
    EmitStatementsWithIndent(stmt->body->statements);

    const ast::Statement* e = stmt->else_statement;
    while (e) {
        if (auto* elseif = e->As<ast::IfStatement>()) {
            {
                line() << "Else IfStatement [";
                {
                    ScopedIndent ifs(this);
                    line() << "condition: [";
                    EmitExpression(elseif->condition);
                }
                line() << "]";
                EmitBlockHeader(elseif->body);
            }
            line() << "]";
            EmitStatementsWithIndent(elseif->body->statements);
            e = elseif->else_statement;
        } else {
            auto* body = e->As<ast::BlockStatement>();
            {
                line() << "Else [";
                {
                    ScopedIndent els(this);
                    EmitBlockHeader(body);
                }
                line() << "]";
            }
            EmitStatementsWithIndent(body->statements);
            break;
        }
    }
}

void GeneratorImpl::EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt) {
    line() << "IncrementDecrementStatement [";
    {
        ScopedIndent ids(this);
        line() << "expr: [";
        EmitExpression(stmt->lhs);
        line() << "]";
        line() << "dir: " << (stmt->increment ? "++" : "--");
    }
    line() << "]";
}

void GeneratorImpl::EmitDiscard(const ast::DiscardStatement*) {
    line() << "DiscardStatement []";
}

void GeneratorImpl::EmitLoop(const ast::LoopStatement* stmt) {
    line() << "LoopStatement [";
    {
        ScopedIndent ls(this);
        EmitStatements(stmt->body->statements);

        if (stmt->continuing && !stmt->continuing->Empty()) {
            line() << "Continuing [";
            {
                ScopedIndent cont(this);
                EmitStatementsWithIndent(stmt->continuing->statements);
            }
            line() << "]";
        }
    }
    line() << "]";
}

void GeneratorImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
    TextBuffer init_buf;
    if (auto* init = stmt->initializer) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
        EmitStatement(init);
    }

    TextBuffer cont_buf;
    if (auto* cont = stmt->continuing) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
        EmitStatement(cont);
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
                    line() << utils::TrimSuffix(init_buf.lines[0].content, ";");
                    break;
                default:  // Block initializer statement
                    for (size_t i = 1; i < init_buf.lines.size(); i++) {
                        // Indent all by the first line
                        init_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    line() << utils::TrimSuffix(init_buf.String(), "\n");
                    break;
            }
        }
        line() << "]";
        line() << "condition: [";
        {
            ScopedIndent con(this);
            if (auto* cond = stmt->condition) {
                EmitExpression(cond);
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
                    line() << utils::TrimSuffix(cont_buf.lines[0].content, ";");
                    break;
                default:  // Block continuing statement
                    for (size_t i = 1; i < cont_buf.lines.size(); i++) {
                        // Indent all by the first line
                        cont_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    line() << utils::TrimSuffix(cont_buf.String(), "\n");
                    break;
            }
        }
        EmitBlockHeader(stmt->body);
        EmitStatementsWithIndent(stmt->body->statements);
    }
    line() << "]";
}

void GeneratorImpl::EmitWhile(const ast::WhileStatement* stmt) {
    line() << "WhileStatement [";
    {
        ScopedIndent ws(this);
        EmitExpression(stmt->condition);
        EmitBlockHeader(stmt->body);
        EmitStatementsWithIndent(stmt->body->statements);
    }
    line() << "]";
}

void GeneratorImpl::EmitReturn(const ast::ReturnStatement* stmt) {
    line() << "ReturnStatement [";
    {
        ScopedIndent ret(this);
        if (stmt->value) {
            EmitExpression(stmt->value);
        }
    }
    line() << "]";
}

void GeneratorImpl::EmitConstAssert(const ast::ConstAssert* stmt) {
    line() << "ConstAssert [";
    {
        ScopedIndent ca(this);
        EmitExpression(stmt->condition);
    }
    line() << "]";
}

void GeneratorImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
    line() << "SwitchStatement [";
    {
        ScopedIndent ss(this);
        line() << "condition: [";
        {
            ScopedIndent cond(this);
            EmitExpression(stmt->condition);
        }
        line() << "]";

        {
            ScopedIndent si(this);
            for (auto* s : stmt->body) {
                EmitCase(s);
            }
        }
    }
    line() << "]";
}

}  // namespace tint::writer::syntax_tree
