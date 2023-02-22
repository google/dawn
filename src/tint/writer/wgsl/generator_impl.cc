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

#include "src/tint/writer/wgsl/generator_impl.h"

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
#include "src/tint/utils/math.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/writer/float_to_string.h"

namespace tint::writer::wgsl {

GeneratorImpl::GeneratorImpl(const Program* program) : TextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
    // Generate directives before any other global declarations.
    bool has_directives = false;
    for (auto enable : program_->AST().Enables()) {
        if (!EmitEnable(enable)) {
            return false;
        }
        has_directives = true;
    }
    for (auto diagnostic : program_->AST().DiagnosticDirectives()) {
        auto out = line();
        if (!EmitDiagnosticControl(out, diagnostic->control)) {
            return false;
        }
        out << ";";
        has_directives = true;
    }
    if (has_directives) {
        line();
    }
    // Generate global declarations in the order they appear in the module.
    for (auto* decl : program_->AST().GlobalDeclarations()) {
        if (decl->IsAnyOf<ast::DiagnosticDirective, ast::Enable>()) {
            continue;
        }
        if (!Switch(
                decl,  //
                [&](const ast::TypeDecl* td) { return EmitTypeDecl(td); },
                [&](const ast::Function* func) { return EmitFunction(func); },
                [&](const ast::Variable* var) { return EmitVariable(line(), var); },
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

bool GeneratorImpl::EmitDiagnosticControl(std::ostream& out,
                                          const ast::DiagnosticControl& diagnostic) {
    out << "diagnostic(" << diagnostic.severity << ", "
        << program_->Symbols().NameFor(diagnostic.rule_name->symbol) << ")";
    return true;
}

bool GeneratorImpl::EmitEnable(const ast::Enable* enable) {
    auto out = line();
    out << "enable " << enable->extension << ";";
    return true;
}

bool GeneratorImpl::EmitTypeDecl(const ast::TypeDecl* ty) {
    return Switch(
        ty,
        [&](const ast::Alias* alias) {  //
            auto out = line();
            out << "alias " << program_->Symbols().NameFor(alias->name->symbol) << " = ";
            if (!EmitExpression(out, alias->type)) {
                return false;
            }
            out << ";";
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

bool GeneratorImpl::EmitExpression(std::ostream& out, const ast::Expression* expr) {
    return Switch(
        expr,
        [&](const ast::IndexAccessorExpression* a) {  //
            return EmitIndexAccessor(out, a);
        },
        [&](const ast::BinaryExpression* b) {  //
            return EmitBinary(out, b);
        },
        [&](const ast::BitcastExpression* b) {  //
            return EmitBitcast(out, b);
        },
        [&](const ast::CallExpression* c) {  //
            return EmitCall(out, c);
        },
        [&](const ast::IdentifierExpression* i) {  //
            return EmitIdentifier(out, i);
        },
        [&](const ast::LiteralExpression* l) {  //
            return EmitLiteral(out, l);
        },
        [&](const ast::MemberAccessorExpression* m) {  //
            return EmitMemberAccessor(out, m);
        },
        [&](const ast::PhonyExpression*) {  //
            out << "_";
            return true;
        },
        [&](const ast::UnaryOpExpression* u) {  //
            return EmitUnaryOp(out, u);
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer, "unknown expression type");
            return false;
        });
}

bool GeneratorImpl::EmitIndexAccessor(std::ostream& out, const ast::IndexAccessorExpression* expr) {
    bool paren_lhs =
        !expr->object
             ->IsAnyOf<ast::AccessorExpression, ast::CallExpression, ast::IdentifierExpression>();
    if (paren_lhs) {
        out << "(";
    }
    if (!EmitExpression(out, expr->object)) {
        return false;
    }
    if (paren_lhs) {
        out << ")";
    }
    out << "[";

    if (!EmitExpression(out, expr->index)) {
        return false;
    }
    out << "]";

    return true;
}

bool GeneratorImpl::EmitMemberAccessor(std::ostream& out,
                                       const ast::MemberAccessorExpression* expr) {
    bool paren_lhs =
        !expr->object
             ->IsAnyOf<ast::AccessorExpression, ast::CallExpression, ast::IdentifierExpression>();
    if (paren_lhs) {
        out << "(";
    }
    if (!EmitExpression(out, expr->object)) {
        return false;
    }
    if (paren_lhs) {
        out << ")";
    }

    out << "." << program_->Symbols().NameFor(expr->member->symbol);
    return true;
}

bool GeneratorImpl::EmitBitcast(std::ostream& out, const ast::BitcastExpression* expr) {
    out << "bitcast<";
    if (!EmitExpression(out, expr->type)) {
        return false;
    }

    out << ">(";
    if (!EmitExpression(out, expr->expr)) {
        return false;
    }

    out << ")";
    return true;
}

bool GeneratorImpl::EmitCall(std::ostream& out, const ast::CallExpression* expr) {
    if (!EmitExpression(out, expr->target)) {
        return false;
    }
    out << "(";

    bool first = true;
    const auto& args = expr->args;
    for (auto* arg : args) {
        if (!first) {
            out << ", ";
        }
        first = false;

        if (!EmitExpression(out, arg)) {
            return false;
        }
    }

    out << ")";

    return true;
}

bool GeneratorImpl::EmitLiteral(std::ostream& out, const ast::LiteralExpression* lit) {
    return Switch(
        lit,
        [&](const ast::BoolLiteralExpression* l) {  //
            out << (l->value ? "true" : "false");
            return true;
        },
        [&](const ast::FloatLiteralExpression* l) {  //
            // f16 literals are also emitted as float value with suffix "h".
            // Note that all normal and subnormal f16 values are normal f32 values, and since NaN
            // and Inf are not allowed to be spelled in literal, it should be fine to emit f16
            // literals in this way.
            if (l->suffix == ast::FloatLiteralExpression::Suffix::kNone) {
                out << DoubleToBitPreservingString(l->value);
            } else {
                out << FloatToBitPreservingString(static_cast<float>(l->value)) << l->suffix;
            }
            return true;
        },
        [&](const ast::IntLiteralExpression* l) {  //
            out << l->value << l->suffix;
            return true;
        },
        [&](Default) {  //
            diagnostics_.add_error(diag::System::Writer, "unknown literal type");
            return false;
        });
}

bool GeneratorImpl::EmitIdentifier(std::ostream& out, const ast::IdentifierExpression* expr) {
    return EmitIdentifier(out, expr->identifier);
}

bool GeneratorImpl::EmitIdentifier(std::ostream& out, const ast::Identifier* ident) {
    if (auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>()) {
        if (!tmpl_ident->attributes.IsEmpty()) {
            EmitAttributes(out, tmpl_ident->attributes);
            out << " ";
        }
        out << program_->Symbols().NameFor(ident->symbol) << "<";
        TINT_DEFER(out << ">");
        for (auto* expr : tmpl_ident->arguments) {
            if (expr != tmpl_ident->arguments.Front()) {
                out << ", ";
            }
            if (!EmitExpression(out, expr)) {
                return false;
            }
        }
    } else {
        out << program_->Symbols().NameFor(ident->symbol);
    }
    return true;
}

bool GeneratorImpl::EmitFunction(const ast::Function* func) {
    if (func->attributes.Length()) {
        if (!EmitAttributes(line(), func->attributes)) {
            return false;
        }
    }
    {
        auto out = line();
        out << "fn " << program_->Symbols().NameFor(func->name->symbol) << "(";

        bool first = true;
        for (auto* v : func->params) {
            if (!first) {
                out << ", ";
            }
            first = false;

            if (!v->attributes.IsEmpty()) {
                if (!EmitAttributes(out, v->attributes)) {
                    return false;
                }
                out << " ";
            }

            out << program_->Symbols().NameFor(v->name->symbol) << " : ";

            if (!EmitExpression(out, v->type)) {
                return false;
            }
        }

        out << ")";

        if (func->return_type || !func->return_type_attributes.IsEmpty()) {
            out << " -> ";

            if (!func->return_type_attributes.IsEmpty()) {
                if (!EmitAttributes(out, func->return_type_attributes)) {
                    return false;
                }
                out << " ";
            }

            if (!EmitExpression(out, func->return_type)) {
                return false;
            }
        }

        if (func->body) {
            out << " ";
            if (!EmitBlockHeader(out, func->body)) {
                return false;
            }
        }
    }

    if (func->body) {
        if (!EmitStatementsWithIndent(func->body->statements)) {
            return false;
        }
        line() << "}";
    }

    return true;
}

bool GeneratorImpl::EmitImageFormat(std::ostream& out, const builtin::TexelFormat fmt) {
    switch (fmt) {
        case builtin::TexelFormat::kUndefined:
            diagnostics_.add_error(diag::System::Writer, "unknown image format");
            return false;
        default:
            out << fmt;
    }
    return true;
}

bool GeneratorImpl::EmitStructType(const ast::Struct* str) {
    if (str->attributes.Length()) {
        if (!EmitAttributes(line(), str->attributes)) {
            return false;
        }
    }
    line() << "struct " << program_->Symbols().NameFor(str->name->symbol) << " {";

    auto add_padding = [&](uint32_t size) {
        line() << "@size(" << size << ")";

        // Note: u32 is the smallest primitive we currently support. When WGSL
        // supports smaller types, this will need to be updated.
        line() << UniqueIdentifier("padding") << " : u32,";
    };

    increment_indent();
    uint32_t offset = 0;
    for (auto* mem : str->members) {
        // TODO(crbug.com/tint/798) move the @offset attribute handling to the transform::Wgsl
        // sanitizer.
        if (auto* mem_sem = program_->Sem().Get(mem)) {
            offset = utils::RoundUp(mem_sem->Align(), offset);
            if (uint32_t padding = mem_sem->Offset() - offset) {
                add_padding(padding);
                offset += padding;
            }
            offset += mem_sem->Size();
        }

        // Offset attributes no longer exist in the WGSL spec, but are emitted
        // by the SPIR-V reader and are consumed by the Resolver(). These should not
        // be emitted, but instead struct padding fields should be emitted.
        utils::Vector<const ast::Attribute*, 4> attributes_sanitized;
        attributes_sanitized.Reserve(mem->attributes.Length());
        for (auto* attr : mem->attributes) {
            if (attr->Is<ast::StructMemberOffsetAttribute>()) {
                auto l = line();
                l << "/* ";
                if (!EmitAttributes(l, utils::Vector{attr})) {
                    return false;
                }
                l << " */";
            } else {
                attributes_sanitized.Push(attr);
            }
        }

        if (!attributes_sanitized.IsEmpty()) {
            if (!EmitAttributes(line(), attributes_sanitized)) {
                return false;
            }
        }

        auto out = line();
        out << program_->Symbols().NameFor(mem->name->symbol) << " : ";
        if (!EmitExpression(out, mem->type)) {
            return false;
        }
        out << ",";
    }
    decrement_indent();

    line() << "}";
    return true;
}

bool GeneratorImpl::EmitVariable(std::ostream& out, const ast::Variable* v) {
    if (!v->attributes.IsEmpty()) {
        if (!EmitAttributes(out, v->attributes)) {
            return false;
        }
        out << " ";
    }

    bool ok = Switch(
        v,  //
        [&](const ast::Var* var) {
            out << "var";
            if (var->declared_address_space || var->declared_access) {
                out << "<";
                TINT_DEFER(out << ">");
                if (!EmitExpression(out, var->declared_address_space)) {
                    return false;
                }
                if (var->declared_access) {
                    out << ", ";
                    if (!EmitExpression(out, var->declared_access)) {
                        return false;
                    }
                }
            }
            return true;
        },
        [&](const ast::Let*) {
            out << "let";
            return true;
        },
        [&](const ast::Override*) {
            out << "override";
            return true;
        },
        [&](const ast::Const*) {
            out << "const";
            return true;
        },
        [&](Default) {
            TINT_ICE(Writer, diagnostics_) << "unhandled variable type " << v->TypeInfo().name;
            return false;
        });
    if (!ok) {
        return false;
    }

    out << " " << program_->Symbols().NameFor(v->name->symbol);

    if (auto ty = v->type) {
        out << " : ";
        if (!EmitExpression(out, ty)) {
            return false;
        }
    }

    if (v->initializer != nullptr) {
        out << " = ";
        if (!EmitExpression(out, v->initializer)) {
            return false;
        }
    }
    out << ";";

    return true;
}

bool GeneratorImpl::EmitAttributes(std::ostream& out,
                                   utils::VectorRef<const ast::Attribute*> attrs) {
    bool first = true;
    for (auto* attr : attrs) {
        if (!first) {
            out << " ";
        }
        first = false;
        out << "@";
        bool ok = Switch(
            attr,
            [&](const ast::WorkgroupAttribute* workgroup) {
                auto values = workgroup->Values();
                out << "workgroup_size(";
                for (size_t i = 0; i < 3; i++) {
                    if (values[i]) {
                        if (i > 0) {
                            out << ", ";
                        }
                        if (!EmitExpression(out, values[i])) {
                            return false;
                        }
                    }
                }
                out << ")";
                return true;
            },
            [&](const ast::StageAttribute* stage) {
                out << stage->stage;
                return true;
            },
            [&](const ast::BindingAttribute* binding) {
                out << "binding(";
                if (!EmitExpression(out, binding->expr)) {
                    return false;
                }
                out << ")";
                return true;
            },
            [&](const ast::GroupAttribute* group) {
                out << "group(";
                if (!EmitExpression(out, group->expr)) {
                    return false;
                }
                out << ")";
                return true;
            },
            [&](const ast::LocationAttribute* location) {
                out << "location(";
                if (!EmitExpression(out, location->expr)) {
                    return false;
                }
                out << ")";
                return true;
            },
            [&](const ast::BuiltinAttribute* builtin) {
                out << "builtin(";
                if (!EmitExpression(out, builtin->builtin)) {
                    return false;
                }
                out << ")";
                return true;
            },
            [&](const ast::DiagnosticAttribute* diagnostic) {
                return EmitDiagnosticControl(out, diagnostic->control);
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                out << "interpolate(";
                if (!EmitExpression(out, interpolate->type)) {
                    return false;
                }
                if (interpolate->sampling) {
                    out << ", ";
                    if (!EmitExpression(out, interpolate->sampling)) {
                        return false;
                    }
                }
                out << ")";
                return true;
            },
            [&](const ast::InvariantAttribute*) {
                out << "invariant";
                return true;
            },
            [&](const ast::IdAttribute* override_deco) {
                out << "id(";
                if (!EmitExpression(out, override_deco->expr)) {
                    return false;
                }
                out << ")";
                return true;
            },
            [&](const ast::MustUseAttribute*) {
                out << "must_use";
                return true;
            },
            [&](const ast::StructMemberOffsetAttribute* offset) {
                out << "offset(";
                if (!EmitExpression(out, offset->expr)) {
                    return false;
                }
                out << ")";
                return true;
            },
            [&](const ast::StructMemberSizeAttribute* size) {
                out << "size(";
                if (!EmitExpression(out, size->expr)) {
                    return false;
                }
                out << ")";
                return true;
            },
            [&](const ast::StructMemberAlignAttribute* align) {
                out << "align(";
                if (!EmitExpression(out, align->expr)) {
                    return false;
                }
                out << ")";
                return true;
            },
            [&](const ast::StrideAttribute* stride) {
                out << "stride(" << stride->stride << ")";
                return true;
            },
            [&](const ast::InternalAttribute* internal) {
                out << "internal(" << internal->InternalName() << ")";
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

bool GeneratorImpl::EmitBinary(std::ostream& out, const ast::BinaryExpression* expr) {
    out << "(";

    if (!EmitExpression(out, expr->lhs)) {
        return false;
    }
    out << " ";
    if (!EmitBinaryOp(out, expr->op)) {
        return false;
    }
    out << " ";

    if (!EmitExpression(out, expr->rhs)) {
        return false;
    }

    out << ")";
    return true;
}

bool GeneratorImpl::EmitBinaryOp(std::ostream& out, const ast::BinaryOp op) {
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
            return false;
    }
    return true;
}

bool GeneratorImpl::EmitUnaryOp(std::ostream& out, const ast::UnaryOpExpression* expr) {
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

    if (!EmitExpression(out, expr->expr)) {
        return false;
    }

    out << ")";

    return true;
}

bool GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
    {
        auto out = line();
        if (!EmitBlockHeader(out, stmt)) {
            return false;
        }
    }
    if (!EmitStatementsWithIndent(stmt->statements)) {
        return false;
    }
    line() << "}";

    return true;
}

bool GeneratorImpl::EmitBlockHeader(std::ostream& out, const ast::BlockStatement* stmt) {
    if (!stmt->attributes.IsEmpty()) {
        if (!EmitAttributes(out, stmt->attributes)) {
            return false;
        }
        out << " ";
    }
    out << "{";
    return true;
}

bool GeneratorImpl::EmitStatement(const ast::Statement* stmt) {
    return Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { return EmitAssign(a); },
        [&](const ast::BlockStatement* b) { return EmitBlock(b); },
        [&](const ast::BreakStatement* b) { return EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { return EmitBreakIf(b); },
        [&](const ast::CallStatement* c) {
            auto out = line();
            if (!EmitCall(out, c->expr)) {
                return false;
            }
            out << ";";
            return true;
        },
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
        [&](const ast::VariableDeclStatement* v) { return EmitVariable(line(), v->variable); },
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
    auto out = line();

    if (!EmitExpression(out, stmt->lhs)) {
        return false;
    }

    out << " = ";

    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }

    out << ";";

    return true;
}

bool GeneratorImpl::EmitBreak(const ast::BreakStatement*) {
    line() << "break;";
    return true;
}

bool GeneratorImpl::EmitBreakIf(const ast::BreakIfStatement* b) {
    auto out = line();

    out << "break if ";
    if (!EmitExpression(out, b->condition)) {
        return false;
    }
    out << ";";
    return true;
}

bool GeneratorImpl::EmitCase(const ast::CaseStatement* stmt) {
    if (stmt->selectors.Length() == 1 && stmt->ContainsDefault()) {
        auto out = line();
        out << "default: ";
        if (!EmitBlockHeader(out, stmt->body)) {
            return false;
        }
    } else {
        auto out = line();
        out << "case ";

        bool first = true;
        for (auto* sel : stmt->selectors) {
            if (!first) {
                out << ", ";
            }

            first = false;

            if (sel->IsDefault()) {
                out << "default";
            } else if (!EmitExpression(out, sel->expr)) {
                return false;
            }
        }
        out << ": ";
        if (!EmitBlockHeader(out, stmt->body)) {
            return false;
        }
    }
    if (!EmitStatementsWithIndent(stmt->body->statements)) {
        return false;
    }

    line() << "}";
    return true;
}

bool GeneratorImpl::EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt) {
    auto out = line();

    if (!EmitExpression(out, stmt->lhs)) {
        return false;
    }

    out << " ";
    if (!EmitBinaryOp(out, stmt->op)) {
        return false;
    }
    out << "= ";

    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }

    out << ";";

    return true;
}

bool GeneratorImpl::EmitContinue(const ast::ContinueStatement*) {
    line() << "continue;";
    return true;
}

bool GeneratorImpl::EmitIf(const ast::IfStatement* stmt) {
    {
        auto out = line();
        out << "if (";
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ") ";
        if (!EmitBlockHeader(out, stmt->body)) {
            return false;
        }
    }

    if (!EmitStatementsWithIndent(stmt->body->statements)) {
        return false;
    }

    const ast::Statement* e = stmt->else_statement;
    while (e) {
        if (auto* elseif = e->As<ast::IfStatement>()) {
            {
                auto out = line();
                out << "} else if (";
                if (!EmitExpression(out, elseif->condition)) {
                    return false;
                }
                out << ") ";
                if (!EmitBlockHeader(out, elseif->body)) {
                    return false;
                }
            }
            if (!EmitStatementsWithIndent(elseif->body->statements)) {
                return false;
            }
            e = elseif->else_statement;
        } else {
            auto* body = e->As<ast::BlockStatement>();
            {
                auto out = line();
                out << "} else ";
                if (!EmitBlockHeader(out, body)) {
                    return false;
                }
            }
            if (!EmitStatementsWithIndent(body->statements)) {
                return false;
            }
            break;
        }
    }

    line() << "}";

    return true;
}

bool GeneratorImpl::EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt) {
    auto out = line();
    if (!EmitExpression(out, stmt->lhs)) {
        return false;
    }
    out << (stmt->increment ? "++" : "--") << ";";
    return true;
}

bool GeneratorImpl::EmitDiscard(const ast::DiscardStatement*) {
    line() << "discard;";
    return true;
}

bool GeneratorImpl::EmitLoop(const ast::LoopStatement* stmt) {
    line() << "loop {";
    increment_indent();

    if (!EmitStatements(stmt->body->statements)) {
        return false;
    }

    if (stmt->continuing && !stmt->continuing->Empty()) {
        line();
        line() << "continuing {";
        if (!EmitStatementsWithIndent(stmt->continuing->statements)) {
            return false;
        }
        line() << "}";
    }

    decrement_indent();
    line() << "}";

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

    {
        auto out = line();
        out << "for";
        {
            ScopedParen sp(out);
            switch (init_buf.lines.size()) {
                case 0:  // No initializer
                    break;
                case 1:  // Single line initializer statement
                    out << TrimSuffix(init_buf.lines[0].content, ";");
                    break;
                default:  // Block initializer statement
                    for (size_t i = 1; i < init_buf.lines.size(); i++) {
                        // Indent all by the first line
                        init_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    out << TrimSuffix(init_buf.String(), "\n");
                    break;
            }

            out << "; ";

            if (auto* cond = stmt->condition) {
                if (!EmitExpression(out, cond)) {
                    return false;
                }
            }

            out << "; ";

            switch (cont_buf.lines.size()) {
                case 0:  // No continuing
                    break;
                case 1:  // Single line continuing statement
                    out << TrimSuffix(cont_buf.lines[0].content, ";");
                    break;
                default:  // Block continuing statement
                    for (size_t i = 1; i < cont_buf.lines.size(); i++) {
                        // Indent all by the first line
                        cont_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    out << TrimSuffix(cont_buf.String(), "\n");
                    break;
            }
        }
        out << " ";
        if (!EmitBlockHeader(out, stmt->body)) {
            return false;
        }
    }

    if (!EmitStatementsWithIndent(stmt->body->statements)) {
        return false;
    }

    line() << "}";

    return true;
}

bool GeneratorImpl::EmitWhile(const ast::WhileStatement* stmt) {
    {
        auto out = line();
        out << "while";
        {
            ScopedParen sp(out);

            auto* cond = stmt->condition;
            if (!EmitExpression(out, cond)) {
                return false;
            }
        }
        out << " ";
        if (!EmitBlockHeader(out, stmt->body)) {
            return false;
        }
    }

    if (!EmitStatementsWithIndent(stmt->body->statements)) {
        return false;
    }

    line() << "}";

    return true;
}

bool GeneratorImpl::EmitReturn(const ast::ReturnStatement* stmt) {
    auto out = line();
    out << "return";
    if (stmt->value) {
        out << " ";
        if (!EmitExpression(out, stmt->value)) {
            return false;
        }
    }
    out << ";";
    return true;
}

bool GeneratorImpl::EmitConstAssert(const ast::ConstAssert* stmt) {
    auto out = line();
    out << "static_assert ";
    if (!EmitExpression(out, stmt->condition)) {
        return false;
    }
    out << ";";
    return true;
}

bool GeneratorImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
    {
        auto out = line();
        out << "switch(";
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ") {";
    }

    {
        ScopedIndent si(this);
        for (auto* s : stmt->body) {
            if (!EmitCase(s)) {
                return false;
            }
        }
    }

    line() << "}";
    return true;
}

}  // namespace tint::writer::wgsl
