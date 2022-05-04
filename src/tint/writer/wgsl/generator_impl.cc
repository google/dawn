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

#include "src/tint/ast/access.h"
#include "src/tint/ast/alias.h"
#include "src/tint/ast/array.h"
#include "src/tint/ast/atomic.h"
#include "src/tint/ast/bool.h"
#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/depth_texture.h"
#include "src/tint/ast/external_texture.h"
#include "src/tint/ast/f32.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/i32.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/matrix.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/multisampled_texture.h"
#include "src/tint/ast/pointer.h"
#include "src/tint/ast/sampled_texture.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/storage_texture.h"
#include "src/tint/ast/stride_attribute.h"
#include "src/tint/ast/struct_member_align_attribute.h"
#include "src/tint/ast/struct_member_offset_attribute.h"
#include "src/tint/ast/struct_member_size_attribute.h"
#include "src/tint/ast/type_name.h"
#include "src/tint/ast/u32.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/vector.h"
#include "src/tint/ast/void.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/sem/struct.h"
#include "src/tint/utils/math.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/writer/float_to_string.h"

namespace tint::writer::wgsl {

GeneratorImpl::GeneratorImpl(const Program* program) : TextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
    // Generate enable directives before any other global declarations.
    for (auto ext : program_->AST().Extensions()) {
        if (!EmitEnableDirective(ext)) {
            return false;
        }
    }
    if (!program_->AST().Extensions().empty()) {
        line();
    }
    // Generate global declarations in the order they appear in the module.
    for (auto* decl : program_->AST().GlobalDeclarations()) {
        if (decl->Is<ast::Enable>()) {
            continue;
        }
        if (!Switch(
                decl,  //
                [&](const ast::TypeDecl* td) { return EmitTypeDecl(td); },
                [&](const ast::Function* func) { return EmitFunction(func); },
                [&](const ast::Variable* var) { return EmitVariable(line(), var); },
                [&](Default) {
                    TINT_UNREACHABLE(Writer, diagnostics_);
                    return false;
                })) {
            return false;
        }
        if (decl != program_->AST().GlobalDeclarations().back()) {
            line();
        }
    }

    return true;
}

bool GeneratorImpl::EmitEnableDirective(const ast::Enable::ExtensionKind ext) {
    auto out = line();
    auto extension = ast::Enable::KindToName(ext);
    if (extension == "") {
        return false;
    }
    out << "enable " << extension << ";";
    return true;
}

bool GeneratorImpl::EmitTypeDecl(const ast::TypeDecl* ty) {
    return Switch(
        ty,
        [&](const ast::Alias* alias) {  //
            auto out = line();
            out << "type " << program_->Symbols().NameFor(alias->name) << " = ";
            if (!EmitType(out, alias->type)) {
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
        !expr->object->IsAnyOf<ast::IndexAccessorExpression, ast::CallExpression,
                               ast::IdentifierExpression, ast::MemberAccessorExpression>();
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
        !expr->structure->IsAnyOf<ast::IndexAccessorExpression, ast::CallExpression,
                                  ast::IdentifierExpression, ast::MemberAccessorExpression>();
    if (paren_lhs) {
        out << "(";
    }
    if (!EmitExpression(out, expr->structure)) {
        return false;
    }
    if (paren_lhs) {
        out << ")";
    }

    out << ".";

    return EmitExpression(out, expr->member);
}

bool GeneratorImpl::EmitBitcast(std::ostream& out, const ast::BitcastExpression* expr) {
    out << "bitcast<";
    if (!EmitType(out, expr->type)) {
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
    if (expr->target.name) {
        if (!EmitExpression(out, expr->target.name)) {
            return false;
        }
    } else if (expr->target.type) {
        if (!EmitType(out, expr->target.type)) {
            return false;
        }
    } else {
        TINT_ICE(Writer, diagnostics_) << "CallExpression target had neither a name or type";
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
            out << FloatToBitPreservingString(l->value);
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
    out << program_->Symbols().NameFor(expr->symbol);
    return true;
}

bool GeneratorImpl::EmitFunction(const ast::Function* func) {
    if (func->attributes.size()) {
        if (!EmitAttributes(line(), func->attributes)) {
            return false;
        }
    }
    {
        auto out = line();
        out << "fn " << program_->Symbols().NameFor(func->symbol) << "(";

        bool first = true;
        for (auto* v : func->params) {
            if (!first) {
                out << ", ";
            }
            first = false;

            if (!v->attributes.empty()) {
                if (!EmitAttributes(out, v->attributes)) {
                    return false;
                }
                out << " ";
            }

            out << program_->Symbols().NameFor(v->symbol) << " : ";

            if (!EmitType(out, v->type)) {
                return false;
            }
        }

        out << ")";

        if (!func->return_type->Is<ast::Void>() || !func->return_type_attributes.empty()) {
            out << " -> ";

            if (!func->return_type_attributes.empty()) {
                if (!EmitAttributes(out, func->return_type_attributes)) {
                    return false;
                }
                out << " ";
            }

            if (!EmitType(out, func->return_type)) {
                return false;
            }
        }

        if (func->body) {
            out << " {";
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

bool GeneratorImpl::EmitImageFormat(std::ostream& out, const ast::TexelFormat fmt) {
    switch (fmt) {
        case ast::TexelFormat::kNone:
            diagnostics_.add_error(diag::System::Writer, "unknown image format");
            return false;
        default:
            out << fmt;
    }
    return true;
}

bool GeneratorImpl::EmitAccess(std::ostream& out, const ast::Access access) {
    switch (access) {
        case ast::Access::kRead:
            out << "read";
            return true;
        case ast::Access::kWrite:
            out << "write";
            return true;
        case ast::Access::kReadWrite:
            out << "read_write";
            return true;
        default:
            break;
    }
    diagnostics_.add_error(diag::System::Writer, "unknown access");
    return false;
}

bool GeneratorImpl::EmitType(std::ostream& out, const ast::Type* ty) {
    return Switch(
        ty,
        [&](const ast::Array* ary) {
            for (auto* attr : ary->attributes) {
                if (auto* stride = attr->As<ast::StrideAttribute>()) {
                    out << "@stride(" << stride->stride << ") ";
                }
            }

            out << "array<";
            if (!EmitType(out, ary->type)) {
                return false;
            }

            if (!ary->IsRuntimeArray()) {
                out << ", ";
                if (!EmitExpression(out, ary->count)) {
                    return false;
                }
            }

            out << ">";
            return true;
        },
        [&](const ast::Bool*) {
            out << "bool";
            return true;
        },
        [&](const ast::F32*) {
            out << "f32";
            return true;
        },
        [&](const ast::I32*) {
            out << "i32";
            return true;
        },
        [&](const ast::Matrix* mat) {
            out << "mat" << mat->columns << "x" << mat->rows;
            if (auto* el_ty = mat->type) {
                out << "<";
                if (!EmitType(out, el_ty)) {
                    return false;
                }
                out << ">";
            }
            return true;
        },
        [&](const ast::Pointer* ptr) {
            out << "ptr<" << ptr->storage_class << ", ";
            if (!EmitType(out, ptr->type)) {
                return false;
            }
            if (ptr->access != ast::Access::kUndefined) {
                out << ", ";
                if (!EmitAccess(out, ptr->access)) {
                    return false;
                }
            }
            out << ">";
            return true;
        },
        [&](const ast::Atomic* atomic) {
            out << "atomic<";
            if (!EmitType(out, atomic->type)) {
                return false;
            }
            out << ">";
            return true;
        },
        [&](const ast::Sampler* sampler) {
            out << "sampler";

            if (sampler->IsComparison()) {
                out << "_comparison";
            }
            return true;
        },
        [&](const ast::ExternalTexture*) {
            out << "texture_external";
            return true;
        },
        [&](const ast::Texture* texture) {
            out << "texture_";
            bool ok = Switch(
                texture,
                [&](const ast::DepthTexture*) {  //
                    out << "depth_";
                    return true;
                },
                [&](const ast::DepthMultisampledTexture*) {  //
                    out << "depth_multisampled_";
                    return true;
                },
                [&](const ast::SampledTexture*) {  //
                    /* nothing to emit */
                    return true;
                },
                [&](const ast::MultisampledTexture*) {  //
                    out << "multisampled_";
                    return true;
                },
                [&](const ast::StorageTexture*) {  //
                    out << "storage_";
                    return true;
                },
                [&](Default) {  //
                    diagnostics_.add_error(diag::System::Writer, "unknown texture type");
                    return false;
                });
            if (!ok) {
                return false;
            }

            switch (texture->dim) {
                case ast::TextureDimension::k1d:
                    out << "1d";
                    break;
                case ast::TextureDimension::k2d:
                    out << "2d";
                    break;
                case ast::TextureDimension::k2dArray:
                    out << "2d_array";
                    break;
                case ast::TextureDimension::k3d:
                    out << "3d";
                    break;
                case ast::TextureDimension::kCube:
                    out << "cube";
                    break;
                case ast::TextureDimension::kCubeArray:
                    out << "cube_array";
                    break;
                default:
                    diagnostics_.add_error(diag::System::Writer, "unknown texture dimension");
                    return false;
            }

            return Switch(
                texture,
                [&](const ast::SampledTexture* sampled) {  //
                    out << "<";
                    if (!EmitType(out, sampled->type)) {
                        return false;
                    }
                    out << ">";
                    return true;
                },
                [&](const ast::MultisampledTexture* ms) {  //
                    out << "<";
                    if (!EmitType(out, ms->type)) {
                        return false;
                    }
                    out << ">";
                    return true;
                },
                [&](const ast::StorageTexture* storage) {  //
                    out << "<";
                    if (!EmitImageFormat(out, storage->format)) {
                        return false;
                    }
                    out << ", ";
                    if (!EmitAccess(out, storage->access)) {
                        return false;
                    }
                    out << ">";
                    return true;
                },
                [&](Default) {  //
                    return true;
                });
        },
        [&](const ast::U32*) {
            out << "u32";
            return true;
        },
        [&](const ast::Vector* vec) {
            out << "vec" << vec->width;
            if (auto* el_ty = vec->type) {
                out << "<";
                if (!EmitType(out, el_ty)) {
                    return false;
                }
                out << ">";
            }
            return true;
        },
        [&](const ast::Void*) {
            out << "void";
            return true;
        },
        [&](const ast::TypeName* tn) {
            out << program_->Symbols().NameFor(tn->name);
            return true;
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown type in EmitType: " + std::string(ty->TypeInfo().name));
            return false;
        });
}

bool GeneratorImpl::EmitStructType(const ast::Struct* str) {
    if (str->attributes.size()) {
        if (!EmitAttributes(line(), str->attributes)) {
            return false;
        }
    }
    line() << "struct " << program_->Symbols().NameFor(str->name) << " {";

    auto add_padding = [&](uint32_t size) {
        line() << "@size(" << size << ")";

        // Note: u32 is the smallest primitive we currently support. When WGSL
        // supports smaller types, this will need to be updated.
        line() << UniqueIdentifier("padding") << " : u32,";
    };

    increment_indent();
    uint32_t offset = 0;
    for (auto* mem : str->members) {
        // TODO(crbug.com/tint/798) move the @offset attribute handling to the
        // transform::Wgsl sanitizer.
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
        ast::AttributeList attributes_sanitized;
        attributes_sanitized.reserve(mem->attributes.size());
        for (auto* attr : mem->attributes) {
            if (!attr->Is<ast::StructMemberOffsetAttribute>()) {
                attributes_sanitized.emplace_back(attr);
            }
        }

        if (!attributes_sanitized.empty()) {
            if (!EmitAttributes(line(), attributes_sanitized)) {
                return false;
            }
        }

        auto out = line();
        out << program_->Symbols().NameFor(mem->symbol) << " : ";
        if (!EmitType(out, mem->type)) {
            return false;
        }
        out << ",";
    }
    decrement_indent();

    line() << "}";
    return true;
}

bool GeneratorImpl::EmitVariable(std::ostream& out, const ast::Variable* var) {
    if (!var->attributes.empty()) {
        if (!EmitAttributes(out, var->attributes)) {
            return false;
        }
        out << " ";
    }

    if (var->is_overridable) {
        out << "override";
    } else if (var->is_const) {
        out << "let";
    } else {
        out << "var";
        auto sc = var->declared_storage_class;
        auto ac = var->declared_access;
        if (sc != ast::StorageClass::kNone || ac != ast::Access::kUndefined) {
            out << "<" << sc;
            if (ac != ast::Access::kUndefined) {
                out << ", ";
                if (!EmitAccess(out, ac)) {
                    return false;
                }
            }
            out << ">";
        }
    }

    out << " " << program_->Symbols().NameFor(var->symbol);

    if (auto* ty = var->type) {
        out << " : ";
        if (!EmitType(out, ty)) {
            return false;
        }
    }

    if (var->constructor != nullptr) {
        out << " = ";
        if (!EmitExpression(out, var->constructor)) {
            return false;
        }
    }
    out << ";";

    return true;
}

bool GeneratorImpl::EmitAttributes(std::ostream& out, const ast::AttributeList& attrs) {
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
                for (int i = 0; i < 3; i++) {
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
                out << "stage(" << stage->stage << ")";
                return true;
            },
            [&](const ast::BindingAttribute* binding) {
                out << "binding(" << binding->value << ")";
                return true;
            },
            [&](const ast::GroupAttribute* group) {
                out << "group(" << group->value << ")";
                return true;
            },
            [&](const ast::LocationAttribute* location) {
                out << "location(" << location->value << ")";
                return true;
            },
            [&](const ast::BuiltinAttribute* builtin) {
                out << "builtin(" << builtin->builtin << ")";
                return true;
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                out << "interpolate(" << interpolate->type;
                if (interpolate->sampling != ast::InterpolationSampling::kNone) {
                    out << ", " << interpolate->sampling;
                }
                out << ")";
                return true;
            },
            [&](const ast::InvariantAttribute*) {
                out << "invariant";
                return true;
            },
            [&](const ast::IdAttribute* override_deco) {
                out << "id(" << override_deco->value << ")";
                return true;
            },
            [&](const ast::StructMemberSizeAttribute* size) {
                out << "size(" << size->size << ")";
                return true;
            },
            [&](const ast::StructMemberAlignAttribute* align) {
                out << "align(" << align->align << ")";
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
    line() << "{";
    if (!EmitStatementsWithIndent(stmt->statements)) {
        return false;
    }
    line() << "}";

    return true;
}

bool GeneratorImpl::EmitStatement(const ast::Statement* stmt) {
    return Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { return EmitAssign(a); },
        [&](const ast::BlockStatement* b) { return EmitBlock(b); },
        [&](const ast::BreakStatement* b) { return EmitBreak(b); },
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
        [&](const ast::FallthroughStatement* f) { return EmitFallthrough(f); },
        [&](const ast::IfStatement* i) { return EmitIf(i); },
        [&](const ast::IncrementDecrementStatement* l) { return EmitIncrementDecrement(l); },
        [&](const ast::LoopStatement* l) { return EmitLoop(l); },
        [&](const ast::ForLoopStatement* l) { return EmitForLoop(l); },
        [&](const ast::ReturnStatement* r) { return EmitReturn(r); },
        [&](const ast::SwitchStatement* s) { return EmitSwitch(s); },
        [&](const ast::VariableDeclStatement* v) { return EmitVariable(line(), v->variable); },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown statement type: " + std::string(stmt->TypeInfo().name));
            return false;
        });
}

bool GeneratorImpl::EmitStatements(const ast::StatementList& stmts) {
    for (auto* s : stmts) {
        if (!EmitStatement(s)) {
            return false;
        }
    }
    return true;
}

bool GeneratorImpl::EmitStatementsWithIndent(const ast::StatementList& stmts) {
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

bool GeneratorImpl::EmitCase(const ast::CaseStatement* stmt) {
    if (stmt->IsDefault()) {
        line() << "default: {";
    } else {
        auto out = line();
        out << "case ";

        bool first = true;
        for (auto* selector : stmt->selectors) {
            if (!first) {
                out << ", ";
            }

            first = false;
            if (!EmitLiteral(out, selector)) {
                return false;
            }
        }
        out << ": {";
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

bool GeneratorImpl::EmitFallthrough(const ast::FallthroughStatement*) {
    line() << "fallthrough;";
    return true;
}

bool GeneratorImpl::EmitIf(const ast::IfStatement* stmt) {
    {
        auto out = line();
        out << "if (";
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ") {";
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
                out << ") {";
            }
            if (!EmitStatementsWithIndent(elseif->body->statements)) {
                return false;
            }
            e = elseif->else_statement;
        } else {
            line() << "} else {";
            if (!EmitStatementsWithIndent(e->As<ast::BlockStatement>()->statements)) {
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
        out << " {";
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
