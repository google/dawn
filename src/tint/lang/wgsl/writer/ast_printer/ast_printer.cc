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

#include "src/tint/lang/wgsl/writer/ast_printer/ast_printer.h"

#include <algorithm>

#include "src/tint/lang/core/builtin/texel_format.h"
#include "src/tint/lang/wgsl/ast/accessor_expression.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/ast/bitcast_expression.h"
#include "src/tint/lang/wgsl/ast/bool_literal_expression.h"
#include "src/tint/lang/wgsl/ast/break_if_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_expression.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/compound_assignment_statement.h"
#include "src/tint/lang/wgsl/ast/const.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/diagnostic_attribute.h"
#include "src/tint/lang/wgsl/ast/diagnostic_rule_name.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/float_literal_expression.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/increment_decrement_statement.h"
#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/index_attribute.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/invariant_attribute.h"
#include "src/tint/lang/wgsl/ast/let.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/member_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/must_use_attribute.h"
#include "src/tint/lang/wgsl/ast/override.h"
#include "src/tint/lang/wgsl/ast/phony_expression.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/stride_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_align_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_offset_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_size_attribute.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/templated_identifier.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/while_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/strconv/float_to_string.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl::writer {

ASTPrinter::ASTPrinter(const Program* program) : program_(program) {}

ASTPrinter::~ASTPrinter() = default;

void ASTPrinter::Generate() {
    // Generate directives before any other global declarations.
    bool has_directives = false;
    for (auto enable : program_->AST().Enables()) {
        EmitEnable(enable);
        has_directives = true;
    }
    for (auto diagnostic : program_->AST().DiagnosticDirectives()) {
        auto out = Line();
        EmitDiagnosticControl(out, diagnostic->control);
        out << ";";
        has_directives = true;
    }
    if (has_directives) {
        Line();
    }
    // Generate global declarations in the order they appear in the module.
    for (auto* decl : program_->AST().GlobalDeclarations()) {
        if (decl->IsAnyOf<ast::DiagnosticDirective, ast::Enable>()) {
            continue;
        }
        Switch(
            decl,  //
            [&](const ast::TypeDecl* td) { return EmitTypeDecl(td); },
            [&](const ast::Function* func) { return EmitFunction(func); },
            [&](const ast::Variable* var) { return EmitVariable(Line(), var); },
            [&](const ast::ConstAssert* ca) { return EmitConstAssert(ca); },
            [&](Default) { TINT_UNREACHABLE(); });
        if (decl != program_->AST().GlobalDeclarations().Back()) {
            Line();
        }
    }
}

void ASTPrinter::EmitDiagnosticControl(StringStream& out,
                                       const ast::DiagnosticControl& diagnostic) {
    out << "diagnostic(" << diagnostic.severity << ", " << diagnostic.rule_name->String() << ")";
}

void ASTPrinter::EmitEnable(const ast::Enable* enable) {
    auto out = Line();
    out << "enable ";
    for (auto* ext : enable->extensions) {
        if (ext != enable->extensions.Front()) {
            out << ", ";
        }
        out << ext->name;
    }
    out << ";";
}

void ASTPrinter::EmitTypeDecl(const ast::TypeDecl* ty) {
    Switch(
        ty,  //
        [&](const ast::Alias* alias) {
            auto out = Line();
            out << "alias " << alias->name->symbol.Name() << " = ";
            EmitExpression(out, alias->type);
            out << ";";
        },
        [&](const ast::Struct* str) { EmitStructType(str); },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown declared type: " + std::string(ty->TypeInfo().name));
        });
}

void ASTPrinter::EmitExpression(StringStream& out, const ast::Expression* expr) {
    Switch(
        expr,  //
        [&](const ast::IndexAccessorExpression* a) { EmitIndexAccessor(out, a); },
        [&](const ast::BinaryExpression* b) { EmitBinary(out, b); },
        [&](const ast::BitcastExpression* b) { EmitBitcast(out, b); },
        [&](const ast::CallExpression* c) { EmitCall(out, c); },
        [&](const ast::IdentifierExpression* i) { EmitIdentifier(out, i); },
        [&](const ast::LiteralExpression* l) { EmitLiteral(out, l); },
        [&](const ast::MemberAccessorExpression* m) { EmitMemberAccessor(out, m); },
        [&](const ast::PhonyExpression*) { out << "_"; },
        [&](const ast::UnaryOpExpression* u) { EmitUnaryOp(out, u); },
        [&](Default) { diagnostics_.add_error(diag::System::Writer, "unknown expression type"); });
}

void ASTPrinter::EmitIndexAccessor(StringStream& out, const ast::IndexAccessorExpression* expr) {
    bool paren_lhs =
        !expr->object
             ->IsAnyOf<ast::AccessorExpression, ast::CallExpression, ast::IdentifierExpression>();
    if (paren_lhs) {
        out << "(";
    }
    EmitExpression(out, expr->object);
    if (paren_lhs) {
        out << ")";
    }
    out << "[";

    EmitExpression(out, expr->index);
    out << "]";
}

void ASTPrinter::EmitMemberAccessor(StringStream& out, const ast::MemberAccessorExpression* expr) {
    bool paren_lhs =
        !expr->object
             ->IsAnyOf<ast::AccessorExpression, ast::CallExpression, ast::IdentifierExpression>();
    if (paren_lhs) {
        out << "(";
    }
    EmitExpression(out, expr->object);
    if (paren_lhs) {
        out << ")";
    }

    out << "." << expr->member->symbol.Name();
}

void ASTPrinter::EmitBitcast(StringStream& out, const ast::BitcastExpression* expr) {
    out << "bitcast<";
    EmitExpression(out, expr->type);

    out << ">(";
    EmitExpression(out, expr->expr);
    out << ")";
}

void ASTPrinter::EmitCall(StringStream& out, const ast::CallExpression* expr) {
    EmitExpression(out, expr->target);
    out << "(";

    bool first = true;
    const auto& args = expr->args;
    for (auto* arg : args) {
        if (!first) {
            out << ", ";
        }
        first = false;

        EmitExpression(out, arg);
    }
    out << ")";
}

void ASTPrinter::EmitLiteral(StringStream& out, const ast::LiteralExpression* lit) {
    Switch(
        lit,  //
        [&](const ast::BoolLiteralExpression* l) { out << (l->value ? "true" : "false"); },
        [&](const ast::FloatLiteralExpression* l) {
            // f16 literals are also emitted as float value with suffix "h".
            // Note that all normal and subnormal f16 values are normal f32 values, and since NaN
            // and Inf are not allowed to be spelled in literal, it should be fine to emit f16
            // literals in this way.
            if (l->suffix == ast::FloatLiteralExpression::Suffix::kNone) {
                out << tint::writer::DoubleToBitPreservingString(l->value);
            } else {
                out << tint::writer::FloatToBitPreservingString(static_cast<float>(l->value))
                    << l->suffix;
            }
        },
        [&](const ast::IntLiteralExpression* l) { out << l->value << l->suffix; },
        [&](Default) { diagnostics_.add_error(diag::System::Writer, "unknown literal type"); });
}

void ASTPrinter::EmitIdentifier(StringStream& out, const ast::IdentifierExpression* expr) {
    EmitIdentifier(out, expr->identifier);
}

void ASTPrinter::EmitIdentifier(StringStream& out, const ast::Identifier* ident) {
    if (auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>()) {
        if (!tmpl_ident->attributes.IsEmpty()) {
            EmitAttributes(out, tmpl_ident->attributes);
            out << " ";
        }
        out << ident->symbol.Name() << "<";
        TINT_DEFER(out << ">");
        for (auto* expr : tmpl_ident->arguments) {
            if (expr != tmpl_ident->arguments.Front()) {
                out << ", ";
            }
            EmitExpression(out, expr);
        }
    } else {
        out << ident->symbol.Name();
    }
}

void ASTPrinter::EmitFunction(const ast::Function* func) {
    if (func->attributes.Length()) {
        EmitAttributes(Line(), func->attributes);
    }
    {
        auto out = Line();
        out << "fn " << func->name->symbol.Name() << "(";

        bool first = true;
        for (auto* v : func->params) {
            if (!first) {
                out << ", ";
            }
            first = false;

            if (!v->attributes.IsEmpty()) {
                EmitAttributes(out, v->attributes);
                out << " ";
            }

            out << v->name->symbol.Name() << " : ";

            EmitExpression(out, v->type);
        }

        out << ")";

        if (func->return_type || !func->return_type_attributes.IsEmpty()) {
            out << " -> ";

            if (!func->return_type_attributes.IsEmpty()) {
                EmitAttributes(out, func->return_type_attributes);
                out << " ";
            }

            EmitExpression(out, func->return_type);
        }

        if (func->body) {
            out << " ";
            EmitBlockHeader(out, func->body);
        }
    }

    if (func->body) {
        EmitStatementsWithIndent(func->body->statements);
        Line() << "}";
    }
}

void ASTPrinter::EmitImageFormat(StringStream& out, const builtin::TexelFormat fmt) {
    switch (fmt) {
        case builtin::TexelFormat::kUndefined:
            diagnostics_.add_error(diag::System::Writer, "unknown image format");
            break;
        default:
            out << fmt;
            break;
    }
}

void ASTPrinter::EmitStructType(const ast::Struct* str) {
    if (str->attributes.Length()) {
        EmitAttributes(Line(), str->attributes);
    }
    Line() << "struct " << str->name->symbol.Name() << " {";

    Hashset<std::string_view, 8> member_names;
    for (auto* mem : str->members) {
        member_names.Add(mem->name->symbol.NameView());
    }
    size_t padding_idx = 0;
    auto new_padding_name = [&] {
        while (true) {
            auto name = "padding_" + tint::ToString(padding_idx++);
            if (member_names.Add(name)) {
                return name;
            }
        }
    };

    auto add_padding = [&](uint32_t size) {
        Line() << "@size(" << size << ")";

        // Note: u32 is the smallest primitive we currently support. When WGSL
        // supports smaller types, this will need to be updated.
        Line() << new_padding_name() << " : u32,";
    };

    IncrementIndent();
    uint32_t offset = 0;
    for (auto* mem : str->members) {
        // TODO(crbug.com/tint/798) move the @offset attribute handling to the transform::Wgsl
        // sanitizer.
        if (auto* mem_sem = program_->Sem().Get(mem)) {
            offset = tint::RoundUp(mem_sem->Align(), offset);
            if (uint32_t padding = mem_sem->Offset() - offset) {
                add_padding(padding);
                offset += padding;
            }
            offset += mem_sem->Size();
        }

        // Offset attributes no longer exist in the WGSL spec, but are emitted
        // by the SPIR-V reader and are consumed by the Resolver(). These should not
        // be emitted, but instead struct padding fields should be emitted.
        Vector<const ast::Attribute*, 4> attributes_sanitized;
        attributes_sanitized.Reserve(mem->attributes.Length());
        for (auto* attr : mem->attributes) {
            if (attr->Is<ast::StructMemberOffsetAttribute>()) {
                auto l = Line();
                l << "/* ";
                EmitAttributes(l, Vector{attr});
                l << " */";
            } else {
                attributes_sanitized.Push(attr);
            }
        }

        if (!attributes_sanitized.IsEmpty()) {
            EmitAttributes(Line(), attributes_sanitized);
        }

        auto out = Line();
        out << mem->name->symbol.Name() << " : ";
        EmitExpression(out, mem->type);
        out << ",";
    }
    DecrementIndent();

    Line() << "}";
}

void ASTPrinter::EmitVariable(StringStream& out, const ast::Variable* v) {
    if (!v->attributes.IsEmpty()) {
        EmitAttributes(out, v->attributes);
        out << " ";
    }

    Switch(
        v,  //
        [&](const ast::Var* var) {
            out << "var";
            if (var->declared_address_space || var->declared_access) {
                out << "<";
                TINT_DEFER(out << ">");
                EmitExpression(out, var->declared_address_space);
                if (var->declared_access) {
                    out << ", ";
                    EmitExpression(out, var->declared_access);
                }
            }
        },
        [&](const ast::Let*) { out << "let"; }, [&](const ast::Override*) { out << "override"; },
        [&](const ast::Const*) { out << "const"; },
        [&](Default) { TINT_ICE() << "unhandled variable type " << v->TypeInfo().name; });

    out << " " << v->name->symbol.Name();

    if (auto ty = v->type) {
        out << " : ";
        EmitExpression(out, ty);
    }

    if (v->initializer != nullptr) {
        out << " = ";
        EmitExpression(out, v->initializer);
    }
    out << ";";
}

void ASTPrinter::EmitAttributes(StringStream& out, VectorRef<const ast::Attribute*> attrs) {
    bool first = true;
    for (auto* attr : attrs) {
        if (!first) {
            out << " ";
        }
        first = false;
        out << "@";
        Switch(
            attr,  //
            [&](const ast::WorkgroupAttribute* workgroup) {
                auto values = workgroup->Values();
                out << "workgroup_size(";
                for (size_t i = 0; i < 3; i++) {
                    if (values[i]) {
                        if (i > 0) {
                            out << ", ";
                        }
                        EmitExpression(out, values[i]);
                    }
                }
                out << ")";
            },
            [&](const ast::StageAttribute* stage) { out << stage->stage; },
            [&](const ast::BindingAttribute* binding) {
                out << "binding(";
                EmitExpression(out, binding->expr);
                out << ")";
            },
            [&](const ast::GroupAttribute* group) {
                out << "group(";
                EmitExpression(out, group->expr);
                out << ")";
            },
            [&](const ast::LocationAttribute* location) {
                out << "location(";
                EmitExpression(out, location->expr);
                out << ")";
            },
            [&](const ast::IndexAttribute* index) {
                out << "index(";
                EmitExpression(out, index->expr);
                out << ")";
            },
            [&](const ast::BuiltinAttribute* builtin) {
                out << "builtin(";
                EmitExpression(out, builtin->builtin);
                out << ")";
            },
            [&](const ast::DiagnosticAttribute* diagnostic) {
                EmitDiagnosticControl(out, diagnostic->control);
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                out << "interpolate(";
                EmitExpression(out, interpolate->type);
                if (interpolate->sampling) {
                    out << ", ";
                    EmitExpression(out, interpolate->sampling);
                }
                out << ")";
            },
            [&](const ast::InvariantAttribute*) { out << "invariant"; },
            [&](const ast::IdAttribute* override_deco) {
                out << "id(";
                EmitExpression(out, override_deco->expr);
                out << ")";
            },
            [&](const ast::MustUseAttribute*) { out << "must_use"; },
            [&](const ast::StructMemberOffsetAttribute* offset) {
                out << "offset(";
                EmitExpression(out, offset->expr);
                out << ")";
            },
            [&](const ast::StructMemberSizeAttribute* size) {
                out << "size(";
                EmitExpression(out, size->expr);
                out << ")";
            },
            [&](const ast::StructMemberAlignAttribute* align) {
                out << "align(";
                EmitExpression(out, align->expr);
                out << ")";
            },
            [&](const ast::StrideAttribute* stride) { out << "stride(" << stride->stride << ")"; },
            [&](const ast::InternalAttribute* internal) {
                out << "internal(" << internal->InternalName() << ")";
            },
            [&](Default) {
                TINT_ICE() << "Unsupported attribute '" << attr->TypeInfo().name << "'";
            });
    }
}

void ASTPrinter::EmitBinary(StringStream& out, const ast::BinaryExpression* expr) {
    out << "(";

    EmitExpression(out, expr->lhs);
    out << " ";
    EmitBinaryOp(out, expr->op);
    out << " ";

    EmitExpression(out, expr->rhs);
    out << ")";
}

void ASTPrinter::EmitBinaryOp(StringStream& out, const ast::BinaryOp op) {
    switch (op) {
        case ast::BinaryOp::kAnd:
            out << "&";
            break;
        case ast::BinaryOp::kOr:
            out << "|";
            break;
        case ast::BinaryOp::kXor:
            out << "^";
            break;
        case ast::BinaryOp::kLogicalAnd:
            out << "&&";
            break;
        case ast::BinaryOp::kLogicalOr:
            out << "||";
            break;
        case ast::BinaryOp::kEqual:
            out << "==";
            break;
        case ast::BinaryOp::kNotEqual:
            out << "!=";
            break;
        case ast::BinaryOp::kLessThan:
            out << "<";
            break;
        case ast::BinaryOp::kGreaterThan:
            out << ">";
            break;
        case ast::BinaryOp::kLessThanEqual:
            out << "<=";
            break;
        case ast::BinaryOp::kGreaterThanEqual:
            out << ">=";
            break;
        case ast::BinaryOp::kShiftLeft:
            out << "<<";
            break;
        case ast::BinaryOp::kShiftRight:
            out << ">>";
            break;
        case ast::BinaryOp::kAdd:
            out << "+";
            break;
        case ast::BinaryOp::kSubtract:
            out << "-";
            break;
        case ast::BinaryOp::kMultiply:
            out << "*";
            break;
        case ast::BinaryOp::kDivide:
            out << "/";
            break;
        case ast::BinaryOp::kModulo:
            out << "%";
            break;
        case ast::BinaryOp::kNone:
            diagnostics_.add_error(diag::System::Writer, "missing binary operation type");
            break;
    }
}

void ASTPrinter::EmitUnaryOp(StringStream& out, const ast::UnaryOpExpression* expr) {
    switch (expr->op) {
        case ast::UnaryOp::kAddressOf:
            out << "&";
            break;
        case ast::UnaryOp::kComplement:
            out << "~";
            break;
        case ast::UnaryOp::kIndirection:
            out << "*";
            break;
        case ast::UnaryOp::kNot:
            out << "!";
            break;
        case ast::UnaryOp::kNegation:
            out << "-";
            break;
    }
    out << "(";
    EmitExpression(out, expr->expr);
    out << ")";
}

void ASTPrinter::EmitBlock(const ast::BlockStatement* stmt) {
    {
        auto out = Line();
        EmitBlockHeader(out, stmt);
    }
    EmitStatementsWithIndent(stmt->statements);
    Line() << "}";
}

void ASTPrinter::EmitBlockHeader(StringStream& out, const ast::BlockStatement* stmt) {
    if (!stmt->attributes.IsEmpty()) {
        EmitAttributes(out, stmt->attributes);
        out << " ";
    }
    out << "{";
}

void ASTPrinter::EmitStatement(const ast::Statement* stmt) {
    Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { EmitAssign(a); },
        [&](const ast::BlockStatement* b) { EmitBlock(b); },
        [&](const ast::BreakStatement* b) { EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { EmitBreakIf(b); },
        [&](const ast::CallStatement* c) {
            auto out = Line();
            EmitCall(out, c->expr);
            out << ";";
        },
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
        [&](const ast::VariableDeclStatement* v) { EmitVariable(Line(), v->variable); },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown statement type: " + std::string(stmt->TypeInfo().name));
        });
}

void ASTPrinter::EmitStatements(VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        EmitStatement(s);
    }
}

void ASTPrinter::EmitStatementsWithIndent(VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    EmitStatements(stmts);
}

void ASTPrinter::EmitAssign(const ast::AssignmentStatement* stmt) {
    auto out = Line();

    EmitExpression(out, stmt->lhs);
    out << " = ";
    EmitExpression(out, stmt->rhs);
    out << ";";
}

void ASTPrinter::EmitBreak(const ast::BreakStatement*) {
    Line() << "break;";
}

void ASTPrinter::EmitBreakIf(const ast::BreakIfStatement* b) {
    auto out = Line();

    out << "break if ";
    EmitExpression(out, b->condition);
    out << ";";
}

void ASTPrinter::EmitCase(const ast::CaseStatement* stmt) {
    if (stmt->selectors.Length() == 1 && stmt->ContainsDefault()) {
        auto out = Line();
        out << "default: ";
        EmitBlockHeader(out, stmt->body);
    } else {
        auto out = Line();
        out << "case ";

        bool first = true;
        for (auto* sel : stmt->selectors) {
            if (!first) {
                out << ", ";
            }

            first = false;

            if (sel->IsDefault()) {
                out << "default";
            } else {
                EmitExpression(out, sel->expr);
            }
        }
        out << ": ";
        EmitBlockHeader(out, stmt->body);
    }
    EmitStatementsWithIndent(stmt->body->statements);
    Line() << "}";
}

void ASTPrinter::EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt) {
    auto out = Line();

    EmitExpression(out, stmt->lhs);
    out << " ";
    EmitBinaryOp(out, stmt->op);
    out << "= ";
    EmitExpression(out, stmt->rhs);
    out << ";";
}

void ASTPrinter::EmitContinue(const ast::ContinueStatement*) {
    Line() << "continue;";
}

void ASTPrinter::EmitIf(const ast::IfStatement* stmt) {
    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "if (";
        EmitExpression(out, stmt->condition);
        out << ") ";
        EmitBlockHeader(out, stmt->body);
    }

    EmitStatementsWithIndent(stmt->body->statements);

    const ast::Statement* e = stmt->else_statement;
    while (e) {
        if (auto* elseif = e->As<ast::IfStatement>()) {
            {
                auto out = Line();
                out << "} else if (";
                EmitExpression(out, elseif->condition);
                out << ") ";
                EmitBlockHeader(out, elseif->body);
            }
            EmitStatementsWithIndent(elseif->body->statements);
            e = elseif->else_statement;
        } else {
            auto* body = e->As<ast::BlockStatement>();
            {
                auto out = Line();
                out << "} else ";
                EmitBlockHeader(out, body);
            }
            EmitStatementsWithIndent(body->statements);
            break;
        }
    }

    Line() << "}";
}

void ASTPrinter::EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt) {
    auto out = Line();
    EmitExpression(out, stmt->lhs);
    out << (stmt->increment ? "++" : "--") << ";";
}

void ASTPrinter::EmitDiscard(const ast::DiscardStatement*) {
    Line() << "discard;";
}

void ASTPrinter::EmitLoop(const ast::LoopStatement* stmt) {
    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "loop ";
        EmitBlockHeader(out, stmt->body);
    }
    IncrementIndent();

    EmitStatements(stmt->body->statements);

    if (stmt->continuing && !stmt->continuing->Empty()) {
        Line();
        {
            auto out = Line();
            out << "continuing ";
            if (!stmt->continuing->attributes.IsEmpty()) {
                EmitAttributes(out, stmt->continuing->attributes);
                out << " ";
            }
            out << "{";
        }
        EmitStatementsWithIndent(stmt->continuing->statements);
        Line() << "}";
    }

    DecrementIndent();
    Line() << "}";
}

void ASTPrinter::EmitForLoop(const ast::ForLoopStatement* stmt) {
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

    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "for";
        {
            ScopedParen sp(out);
            switch (init_buf.lines.size()) {
                case 0:  // No initializer
                    break;
                case 1:  // Single line initializer statement
                    out << tint::TrimSuffix(init_buf.lines[0].content, ";");
                    break;
                default:  // Block initializer statement
                    for (size_t i = 1; i < init_buf.lines.size(); i++) {
                        // Indent all by the first line
                        init_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    out << tint::TrimSuffix(init_buf.String(), "\n");
                    break;
            }

            out << "; ";

            if (auto* cond = stmt->condition) {
                EmitExpression(out, cond);
            }

            out << "; ";

            switch (cont_buf.lines.size()) {
                case 0:  // No continuing
                    break;
                case 1:  // Single line continuing statement
                    out << tint::TrimSuffix(cont_buf.lines[0].content, ";");
                    break;
                default:  // Block continuing statement
                    for (size_t i = 1; i < cont_buf.lines.size(); i++) {
                        // Indent all by the first line
                        cont_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    out << tint::TrimSuffix(cont_buf.String(), "\n");
                    break;
            }
        }
        out << " ";
        EmitBlockHeader(out, stmt->body);
    }

    EmitStatementsWithIndent(stmt->body->statements);

    Line() << "}";
}

void ASTPrinter::EmitWhile(const ast::WhileStatement* stmt) {
    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "while";
        {
            ScopedParen sp(out);

            auto* cond = stmt->condition;
            EmitExpression(out, cond);
        }
        out << " ";
        EmitBlockHeader(out, stmt->body);
    }

    EmitStatementsWithIndent(stmt->body->statements);

    Line() << "}";
}

void ASTPrinter::EmitReturn(const ast::ReturnStatement* stmt) {
    auto out = Line();

    out << "return";
    if (stmt->value) {
        out << " ";
        EmitExpression(out, stmt->value);
    }
    out << ";";
}

void ASTPrinter::EmitConstAssert(const ast::ConstAssert* stmt) {
    auto out = Line();
    out << "const_assert ";
    EmitExpression(out, stmt->condition);
    out << ";";
}

void ASTPrinter::EmitSwitch(const ast::SwitchStatement* stmt) {
    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "switch(";
        EmitExpression(out, stmt->condition);
        out << ") ";

        if (!stmt->body_attributes.IsEmpty()) {
            EmitAttributes(out, stmt->body_attributes);
            out << " ";
        }

        out << "{";
    }

    {
        ScopedIndent si(this);
        for (auto* s : stmt->body) {
            EmitCase(s);
        }
    }

    Line() << "}";
}

}  // namespace tint::wgsl::writer
