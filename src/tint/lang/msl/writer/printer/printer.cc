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

#include "src/tint/lang/msl/writer/printer/printer.h"

#include <utility>

#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/msl/writer/common/printer_support.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"

namespace tint::msl::writer {
namespace {

void Sanitize(ir::Module*) {}

}  // namespace

// Helper for calling TINT_UNIMPLEMENTED() from a Switch(object_ptr) default case.
#define UNHANDLED_CASE(object_ptr)                         \
    TINT_UNIMPLEMENTED() << "unhandled case in Switch(): " \
                         << (object_ptr ? object_ptr->TypeInfo().name : "<null>")

Printer::Printer(ir::Module* module) : ir_(module) {}

Printer::~Printer() = default;

bool Printer::Generate() {
    auto valid = ir::Validate(*ir_);
    if (!valid) {
        diagnostics_ = valid.Failure();
        return false;
    }

    // Run the IR transformations to prepare for MSL emission.
    Sanitize(ir_);

    {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
        Line() << "#include <metal_stdlib>";
        Line() << "using namespace metal;";
    }

    // Emit module-scope declarations.
    if (ir_->root_block) {
        EmitBlockInstructions(ir_->root_block);
    }

    // Emit functions.
    for (auto* func : ir_->functions) {
        EmitFunction(func);
    }

    if (diagnostics_.contains_errors()) {
        return false;
    }

    return true;
}

std::string Printer::Result() const {
    StringStream ss;
    ss << preamble_buffer_.String() << std::endl << main_buffer_.String();
    return ss.str();
}

const std::string& Printer::ArrayTemplateName() {
    if (!array_template_name_.empty()) {
        return array_template_name_;
    }

    array_template_name_ = UniqueIdentifier("tint_array");

    TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
    Line() << "template<typename T, size_t N>";
    Line() << "struct " << array_template_name_ << " {";

    {
        ScopedIndent si(current_buffer_);
        Line() << "const constant T& operator[](size_t i) const constant { return elements[i]; }";
        for (auto* space : {"device", "thread", "threadgroup"}) {
            Line() << space << " T& operator[](size_t i) " << space << " { return elements[i]; }";
            Line() << "const " << space << " T& operator[](size_t i) const " << space
                   << " { return elements[i]; }";
        }
        Line() << "T elements[N];";
    }
    Line() << "};";
    Line();

    return array_template_name_;
}

void Printer::EmitFunction(ir::Function* func) {
    TINT_SCOPED_ASSIGNMENT(current_function_, func);

    {
        auto out = Line();

        // TODO(dsinclair): Emit function stage if any
        // TODO(dsinclair): Handle return type attributes

        EmitType(out, func->ReturnType());
        out << " " << ir_->NameOf(func).Name() << "() {";

        // TODO(dsinclair): Emit Function parameters
    }
    {
        ScopedIndent si(current_buffer_);
        EmitBlock(func->Block());
    }

    Line() << "}";
}

void Printer::EmitBlock(ir::Block* block) {
    MarkInlinable(block);

    EmitBlockInstructions(block);
}

void Printer::EmitBlockInstructions(ir::Block* block) {
    TINT_SCOPED_ASSIGNMENT(current_block_, block);

    for (auto* inst : *block) {
        Switch(
            inst,                                          //
            [&](ir::Binary* b) { EmitBinary(b); },         //
            [&](ir::ExitIf* e) { EmitExitIf(e); },         //
            [&](ir::If* if_) { EmitIf(if_); },             //
            [&](ir::Let* l) { EmitLet(l); },               //
            [&](ir::Load* l) { EmitLoad(l); },             //
            [&](ir::Return* r) { EmitReturn(r); },         //
            [&](ir::Unreachable*) { EmitUnreachable(); },  //
            [&](ir::Var* v) { EmitVar(v); },               //
            [&](Default) { TINT_ICE() << "unimplemented instruction: " << inst->TypeInfo().name; });
    }
}

void Printer::EmitBinary(ir::Binary* b) {
    if (b->Kind() == ir::Binary::Kind::kEqual) {
        auto* rhs = b->RHS()->As<ir::Constant>();
        if (rhs && rhs->Type()->Is<type::Bool>() && rhs->Value()->ValueAs<bool>() == false) {
            // expr == false
            Bind(b->Result(), "!(" + Expr(b->LHS()) + ")");
            return;
        }
    }

    auto kind = [&] {
        switch (b->Kind()) {
            case ir::Binary::Kind::kAdd:
                return "+";
            case ir::Binary::Kind::kSubtract:
                return "-";
            case ir::Binary::Kind::kMultiply:
                return "*";
            case ir::Binary::Kind::kDivide:
                return "/";
            case ir::Binary::Kind::kModulo:
                return "%";
            case ir::Binary::Kind::kAnd:
                return "&";
            case ir::Binary::Kind::kOr:
                return "|";
            case ir::Binary::Kind::kXor:
                return "^";
            case ir::Binary::Kind::kEqual:
                return "==";
            case ir::Binary::Kind::kNotEqual:
                return "!=";
            case ir::Binary::Kind::kLessThan:
                return "<";
            case ir::Binary::Kind::kGreaterThan:
                return ">";
            case ir::Binary::Kind::kLessThanEqual:
                return "<=";
            case ir::Binary::Kind::kGreaterThanEqual:
                return ">=";
            case ir::Binary::Kind::kShiftLeft:
                return "<<";
            case ir::Binary::Kind::kShiftRight:
                return ">>";
        }
        return "<error>";
    };

    StringStream str;
    str << "(" << Expr(b->LHS()) << " " << kind() << " " + Expr(b->RHS()) << ")";

    Bind(b->Result(), str.str());
}

void Printer::EmitLoad(ir::Load* l) {
    // Force loads to be bound as inlines
    bindings_.Add(l->Result(), InlinedValue{Expr(l->From()), PtrKind::kRef});
}

void Printer::EmitVar(ir::Var* v) {
    auto out = Line();

    auto* ptr = v->Result()->Type()->As<type::Pointer>();
    TINT_ASSERT_OR_RETURN(ptr);

    auto space = ptr->AddressSpace();
    switch (space) {
        case builtin::AddressSpace::kFunction:
        case builtin::AddressSpace::kHandle:
            break;
        case builtin::AddressSpace::kPrivate:
            out << "thread ";
            break;
        case builtin::AddressSpace::kWorkgroup:
            out << "threadgroup ";
            break;
        default:
            TINT_ICE() << "unhandled variable address space";
            return;
    }

    auto name = ir_->NameOf(v);

    EmitType(out, ptr->UnwrapPtr());
    out << " " << name.Name();

    if (v->Initializer()) {
        out << " = " << Expr(v->Initializer());
    } else if (space == builtin::AddressSpace::kPrivate ||
               space == builtin::AddressSpace::kFunction ||
               space == builtin::AddressSpace::kUndefined) {
        out << " = ";
        EmitZeroValue(out, ptr->UnwrapPtr());
    }
    out << ";";

    Bind(v->Result(), name, PtrKind::kRef);
}

void Printer::EmitLet(ir::Let* l) {
    Bind(l->Result(), Expr(l->Value(), PtrKind::kPtr), PtrKind::kPtr);
}

void Printer::EmitIf(ir::If* if_) {
    // Emit any nodes that need to be used as PHI nodes
    for (auto* phi : if_->Results()) {
        if (!ir_->NameOf(phi).IsValid()) {
            ir_->SetName(phi, ir_->symbols.New());
        }

        auto name = ir_->NameOf(phi);

        auto out = Line();
        EmitType(out, phi->Type());
        out << " " << name.Name() << ";";

        Bind(phi, name);
    }

    Line() << "if (" << Expr(if_->Condition()) << ") {";

    {
        ScopedIndent si(current_buffer_);
        EmitBlockInstructions(if_->True());
    }

    if (if_->False() && !if_->False()->IsEmpty()) {
        Line() << "} else {";

        ScopedIndent si(current_buffer_);
        EmitBlockInstructions(if_->False());
    }

    Line() << "}";
}

void Printer::EmitExitIf(ir::ExitIf* e) {
    auto results = e->If()->Results();
    auto args = e->Args();
    for (size_t i = 0; i < e->Args().Length(); ++i) {
        auto* phi = results[i];
        auto* val = args[i];

        Line() << ir_->NameOf(phi).Name() << " = " << Expr(val) << ";";
    }
}

void Printer::EmitReturn(ir::Return* r) {
    // If this return has no arguments and the current block is for the function which is
    // being returned, skip the return.
    if (current_block_ == current_function_->Block() && r->Args().IsEmpty()) {
        return;
    }

    auto out = Line();
    out << "return";
    if (!r->Args().IsEmpty()) {
        out << " " << Expr(r->Args().Front());
    }
    out << ";";
}

void Printer::EmitUnreachable() {
    Line() << "/* unreachable */";
}

void Printer::EmitAddressSpace(StringStream& out, builtin::AddressSpace sc) {
    switch (sc) {
        case builtin::AddressSpace::kFunction:
        case builtin::AddressSpace::kPrivate:
        case builtin::AddressSpace::kHandle:
            out << "thread";
            break;
        case builtin::AddressSpace::kWorkgroup:
            out << "threadgroup";
            break;
        case builtin::AddressSpace::kStorage:
            out << "device";
            break;
        case builtin::AddressSpace::kUniform:
            out << "constant";
            break;
        default:
            TINT_ICE() << "unhandled address space: " << sc;
            break;
    }
}

void Printer::EmitType(StringStream& out, const type::Type* ty) {
    tint::Switch(
        ty,                                         //
        [&](const type::Bool*) { out << "bool"; },  //
        [&](const type::Void*) { out << "void"; },  //
        [&](const type::F32*) { out << "float"; },  //
        [&](const type::F16*) { out << "half"; },   //
        [&](const type::I32*) { out << "int"; },    //
        [&](const type::U32*) { out << "uint"; },   //
        [&](const type::Array* arr) { EmitArrayType(out, arr); },
        [&](const type::Vector* vec) { EmitVectorType(out, vec); },
        [&](const type::Matrix* mat) { EmitMatrixType(out, mat); },
        [&](const type::Atomic* atomic) { EmitAtomicType(out, atomic); },
        [&](const type::Pointer* ptr) { EmitPointerType(out, ptr); },
        [&](const type::Sampler*) { out << "sampler"; },  //
        [&](const type::Texture* tex) { EmitTextureType(out, tex); },
        [&](const type::Struct* str) {
            out << StructName(str);

            TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
            EmitStructType(str);
        },
        [&](Default) { UNHANDLED_CASE(ty); });
}

void Printer::EmitPointerType(StringStream& out, const type::Pointer* ptr) {
    if (ptr->Access() == builtin::Access::kRead) {
        out << "const ";
    }
    EmitAddressSpace(out, ptr->AddressSpace());
    out << " ";
    EmitType(out, ptr->StoreType());
    out << "*";
}

void Printer::EmitAtomicType(StringStream& out, const type::Atomic* atomic) {
    if (atomic->Type()->Is<type::I32>()) {
        out << "atomic_int";
        return;
    }
    if (TINT_LIKELY(atomic->Type()->Is<type::U32>())) {
        out << "atomic_uint";
        return;
    }
    TINT_ICE() << "unhandled atomic type " << atomic->Type()->FriendlyName();
}

void Printer::EmitArrayType(StringStream& out, const type::Array* arr) {
    out << ArrayTemplateName() << "<";
    EmitType(out, arr->ElemType());
    out << ", ";
    if (arr->Count()->Is<type::RuntimeArrayCount>()) {
        out << "1";
    } else {
        auto count = arr->ConstantCount();
        if (!count) {
            diagnostics_.add_error(diag::System::Writer, type::Array::kErrExpectedConstantCount);
            return;
        }
        out << count.value();
    }
    out << ">";
}

void Printer::EmitVectorType(StringStream& out, const type::Vector* vec) {
    if (vec->Packed()) {
        out << "packed_";
    }
    EmitType(out, vec->type());
    out << vec->Width();
}

void Printer::EmitMatrixType(StringStream& out, const type::Matrix* mat) {
    EmitType(out, mat->type());
    out << mat->columns() << "x" << mat->rows();
}

void Printer::EmitTextureType(StringStream& out, const type::Texture* tex) {
    if (TINT_UNLIKELY(tex->Is<type::ExternalTexture>())) {
        TINT_ICE() << "Multiplanar external texture transform was not run.";
        return;
    }

    if (tex->IsAnyOf<type::DepthTexture, type::DepthMultisampledTexture>()) {
        out << "depth";
    } else {
        out << "texture";
    }

    switch (tex->dim()) {
        case type::TextureDimension::k1d:
            out << "1d";
            break;
        case type::TextureDimension::k2d:
            out << "2d";
            break;
        case type::TextureDimension::k2dArray:
            out << "2d_array";
            break;
        case type::TextureDimension::k3d:
            out << "3d";
            break;
        case type::TextureDimension::kCube:
            out << "cube";
            break;
        case type::TextureDimension::kCubeArray:
            out << "cube_array";
            break;
        default:
            diagnostics_.add_error(diag::System::Writer, "Invalid texture dimensions");
            return;
    }
    if (tex->IsAnyOf<type::MultisampledTexture, type::DepthMultisampledTexture>()) {
        out << "_ms";
    }
    out << "<";
    TINT_DEFER(out << ">");

    tint::Switch(
        tex,  //
        [&](const type::DepthTexture*) { out << "float, access::sample"; },
        [&](const type::DepthMultisampledTexture*) { out << "float, access::read"; },
        [&](const type::StorageTexture* storage) {
            EmitType(out, storage->type());
            out << ", ";

            std::string access_str;
            if (storage->access() == builtin::Access::kRead) {
                out << "access::read";
            } else if (storage->access() == builtin::Access::kWrite) {
                out << "access::write";
            } else {
                diagnostics_.add_error(diag::System::Writer,
                                       "Invalid access control for storage texture");
                return;
            }
        },
        [&](const type::MultisampledTexture* ms) {
            EmitType(out, ms->type());
            out << ", access::read";
        },
        [&](const type::SampledTexture* sampled) {
            EmitType(out, sampled->type());
            out << ", access::sample";
        },
        [&](Default) { diagnostics_.add_error(diag::System::Writer, "invalid texture type"); });
}

void Printer::EmitStructType(const type::Struct* str) {
    auto it = emitted_structs_.emplace(str);
    if (!it.second) {
        return;
    }

    // This does not append directly to the preamble because a struct may require other
    // structs, or the array template, to get emitted before it. So, the struct emits into a
    // temporary text buffer, then anything it depends on will emit to the preamble first,
    // and then it copies the text buffer into the preamble.
    TextBuffer str_buf;
    Line(&str_buf) << "struct " << StructName(str) << " {";

    bool is_host_shareable = str->IsHostShareable();

    // Emits a `/* 0xnnnn */` byte offset comment for a struct member.
    auto add_byte_offset_comment = [&](StringStream& out, uint32_t offset) {
        std::ios_base::fmtflags saved_flag_state(out.flags());
        out << "/* 0x" << std::hex << std::setfill('0') << std::setw(4) << offset << " */ ";
        out.flags(saved_flag_state);
    };

    auto add_padding = [&](uint32_t size, uint32_t msl_offset) {
        std::string name;
        do {
            name = UniqueIdentifier("tint_pad");
        } while (str->FindMember(ir_->symbols.Get(name)));

        auto out = Line(&str_buf);
        add_byte_offset_comment(out, msl_offset);
        out << ArrayTemplateName() << "<int8_t, " << size << "> " << name << ";";
    };

    str_buf.IncrementIndent();

    uint32_t msl_offset = 0;
    for (auto* mem : str->Members()) {
        auto out = Line(&str_buf);
        auto mem_name = mem->Name().Name();
        auto ir_offset = mem->Offset();

        if (is_host_shareable) {
            if (TINT_UNLIKELY(ir_offset < msl_offset)) {
                // Unimplementable layout
                TINT_ICE() << "Structure member offset (" << ir_offset << ") is behind MSL offset ("
                           << msl_offset << ")";
                return;
            }

            // Generate padding if required
            if (auto padding = ir_offset - msl_offset) {
                add_padding(padding, msl_offset);
                msl_offset += padding;
            }

            add_byte_offset_comment(out, msl_offset);
        }

        auto* ty = mem->Type();

        EmitType(out, ty);
        out << " " << mem_name;

        // Emit attributes
        auto& attributes = mem->Attributes();

        if (auto builtin = attributes.builtin) {
            auto name = BuiltinToAttribute(builtin.value());
            if (name.empty()) {
                diagnostics_.add_error(diag::System::Writer, "unknown builtin");
                return;
            }
            out << " [[" << name << "]]";
        }

        if (auto location = attributes.location) {
            auto& pipeline_stage_uses = str->PipelineStageUses();
            if (TINT_UNLIKELY(pipeline_stage_uses.size() != 1)) {
                TINT_ICE() << "invalid entry point IO struct uses";
                return;
            }

            if (pipeline_stage_uses.count(type::PipelineStageUsage::kVertexInput)) {
                out << " [[attribute(" + std::to_string(location.value()) + ")]]";
            } else if (pipeline_stage_uses.count(type::PipelineStageUsage::kVertexOutput)) {
                out << " [[user(locn" + std::to_string(location.value()) + ")]]";
            } else if (pipeline_stage_uses.count(type::PipelineStageUsage::kFragmentInput)) {
                out << " [[user(locn" + std::to_string(location.value()) + ")]]";
            } else if (TINT_LIKELY(
                           pipeline_stage_uses.count(type::PipelineStageUsage::kFragmentOutput))) {
                out << " [[color(" + std::to_string(location.value()) + ")]]";
            } else {
                TINT_ICE() << "invalid use of location decoration";
                return;
            }
        }

        if (auto interpolation = attributes.interpolation) {
            auto name = InterpolationToAttribute(interpolation->type, interpolation->sampling);
            if (name.empty()) {
                diagnostics_.add_error(diag::System::Writer, "unknown interpolation attribute");
                return;
            }
            out << " [[" << name << "]]";
        }

        if (attributes.invariant) {
            invariant_define_name_ = UniqueIdentifier("TINT_INVARIANT");
            out << " " << invariant_define_name_;
        }

        out << ";";

        if (is_host_shareable) {
            // Calculate new MSL offset
            auto size_align = MslPackedTypeSizeAndAlign(diagnostics_, ty);
            if (TINT_UNLIKELY(msl_offset % size_align.align)) {
                TINT_ICE() << "Misaligned MSL structure member " << mem_name << " : "
                           << ty->FriendlyName() << " offset: " << msl_offset
                           << " align: " << size_align.align;
                return;
            }
            msl_offset += size_align.size;
        }
    }

    if (is_host_shareable && str->Size() != msl_offset) {
        add_padding(str->Size() - msl_offset, msl_offset);
    }

    str_buf.DecrementIndent();
    Line(&str_buf) << "};";

    preamble_buffer_.Append(str_buf);
}

void Printer::EmitConstant(StringStream& out, ir::Constant* c) {
    EmitConstant(out, c->Value());
}

void Printer::EmitConstant(StringStream& out, const constant::Value* c) {
    auto emit_values = [&](uint32_t count) {
        for (size_t i = 0; i < count; i++) {
            if (i > 0) {
                out << ", ";
            }
            EmitConstant(out, c->Index(i));
        }
    };

    tint::Switch(
        c->Type(),  //
        [&](const type::Bool*) { out << (c->ValueAs<bool>() ? "true" : "false"); },
        [&](const type::I32*) { PrintI32(out, c->ValueAs<i32>()); },
        [&](const type::U32*) { out << c->ValueAs<u32>() << "u"; },
        [&](const type::F32*) { PrintF32(out, c->ValueAs<f32>()); },
        [&](const type::F16*) { PrintF16(out, c->ValueAs<f16>()); },
        [&](const type::Vector* v) {
            EmitType(out, v);

            ScopedParen sp(out);
            if (auto* splat = c->As<constant::Splat>()) {
                EmitConstant(out, splat->el);
                return;
            }
            emit_values(v->Width());
        },
        [&](const type::Matrix* m) {
            EmitType(out, m);
            ScopedParen sp(out);
            emit_values(m->columns());
        },
        [&](const type::Array* a) {
            EmitType(out, a);
            out << "{";
            TINT_DEFER(out << "}");

            if (c->AllZero()) {
                return;
            }

            auto count = a->ConstantCount();
            if (!count) {
                diagnostics_.add_error(diag::System::Writer,
                                       type::Array::kErrExpectedConstantCount);
                return;
            }
            emit_values(*count);
        },
        [&](const type::Struct* s) {
            EmitStructType(s);
            out << StructName(s) << "{";
            TINT_DEFER(out << "}");

            if (c->AllZero()) {
                return;
            }

            auto members = s->Members();
            for (size_t i = 0; i < members.Length(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                out << "." << members[i]->Name().Name() << "=";
                EmitConstant(out, c->Index(i));
            }
        },
        [&](Default) { UNHANDLED_CASE(c->Type()); });
}

void Printer::EmitZeroValue(StringStream& out, const type::Type* ty) {
    Switch(
        ty, [&](const type::Bool*) { out << "false"; },                     //
        [&](const type::F16*) { out << "0.0h"; },                           //
        [&](const type::F32*) { out << "0.0f"; },                           //
        [&](const type::I32*) { out << "0"; },                              //
        [&](const type::U32*) { out << "0u"; },                             //
        [&](const type::Vector* vec) { EmitZeroValue(out, vec->type()); },  //
        [&](const type::Matrix* mat) {
            EmitType(out, mat);

            ScopedParen sp(out);
            EmitZeroValue(out, mat->type());
        },
        [&](const type::Array*) { out << "{}"; },   //
        [&](const type::Struct*) { out << "{}"; },  //
        [&](Default) { TINT_ICE() << "Invalid type for zero emission: " << ty->FriendlyName(); });
}

std::string Printer::StructName(const type::Struct* s) {
    auto name = s->Name().Name();
    if (HasPrefix(name, "__")) {
        name = tint::GetOrCreate(builtin_struct_names_, s,
                                 [&] { return UniqueIdentifier(name.substr(2)); });
    }
    return name;
}

std::string Printer::UniqueIdentifier(const std::string& prefix /* = "" */) {
    return ir_->symbols.New(prefix).Name();
}

TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);

std::string Printer::Expr(ir::Value* value, PtrKind want_ptr_kind) {
    using ExprAndPtrKind = std::pair<std::string, PtrKind>;

    auto [expr, got_ptr_kind] = tint::Switch(
        value,
        [&](ir::Constant* c) -> ExprAndPtrKind {
            StringStream str;
            EmitConstant(str, c);
            return {str.str(), PtrKind::kRef};
        },
        [&](Default) -> ExprAndPtrKind {
            auto lookup = bindings_.Find(value);
            if (TINT_UNLIKELY(!lookup)) {
                TINT_ICE() << "Expr(" << (value ? value->TypeInfo().name : "null")
                           << ") value has no expression";
                return {};
            }

            return std::visit(
                [&](auto&& got) -> ExprAndPtrKind {
                    using T = std::decay_t<decltype(got)>;

                    if constexpr (std::is_same_v<T, VariableValue>) {
                        return {got.name.Name(), got.ptr_kind};
                    }

                    if constexpr (std::is_same_v<T, InlinedValue>) {
                        auto result = ExprAndPtrKind{got.expr, got.ptr_kind};

                        // Single use (inlined) expression.
                        // Mark the bindings_ map entry as consumed.
                        *lookup = ConsumedValue{};
                        return result;
                    }

                    if constexpr (std::is_same_v<T, ConsumedValue>) {
                        TINT_ICE() << "Expr(" << value->TypeInfo().name
                                   << ") called twice on the same value";
                    } else {
                        TINT_ICE() << "Expr(" << value->TypeInfo().name << ") has unhandled value";
                    }
                    return {};
                },
                *lookup);
        });
    if (expr.empty()) {
        return "<error>";
    }

    if (value->Type()->Is<type::Pointer>()) {
        return ToPtrKind(expr, got_ptr_kind, want_ptr_kind);
    }

    return expr;
}

TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);

std::string Printer::ToPtrKind(const std::string& in, PtrKind got, PtrKind want) {
    if (want == PtrKind::kRef && got == PtrKind::kPtr) {
        return "*(" + in + ")";
    }
    if (want == PtrKind::kPtr && got == PtrKind::kRef) {
        return "&(" + in + ")";
    }
    return in;
}

void Printer::Bind(ir::Value* value, const std::string& expr, PtrKind ptr_kind) {
    TINT_ASSERT(value);

    if (can_inline_.Remove(value)) {
        // Value will be inlined at its place of usage.
        if (TINT_LIKELY(bindings_.Add(value, InlinedValue{expr, ptr_kind}))) {
            return;
        }
    } else {
        auto mod_name = ir_->NameOf(value);
        if (value->Usages().IsEmpty() && !mod_name.IsValid()) {
            // Drop phonies.
        } else {
            if (mod_name.Name().empty()) {
                mod_name = ir_->symbols.New("v");
            }

            auto out = Line();
            EmitType(out, value->Type());
            out << " const " << mod_name.Name() << " = ";
            if (value->Type()->Is<type::Pointer>()) {
                out << ToPtrKind(expr, ptr_kind, PtrKind::kPtr);
            } else {
                out << expr;
            }
            out << ";";

            Bind(value, mod_name, PtrKind::kPtr);
        }
        return;
    }

    TINT_ICE() << "Bind(" << value->TypeInfo().name << ") called twice for same value";
}

void Printer::Bind(ir::Value* value, Symbol name, PtrKind ptr_kind) {
    TINT_ASSERT(value);

    bool added = bindings_.Add(value, VariableValue{name, ptr_kind});
    if (TINT_UNLIKELY(!added)) {
        TINT_ICE() << "Bind(" << value->TypeInfo().name << ") called twice for same value";
    }
}

void Printer::MarkInlinable(ir::Block* block) {
    // An ordered list of possibly-inlinable values returned by sequenced instructions that have
    // not yet been marked-for or ruled-out-for inlining.
    UniqueVector<ir::Value*, 32> pending_resolution;

    // Walk the instructions of the block starting with the first.
    for (auto* inst : *block) {
        // Is the instruction sequenced?
        bool sequenced = inst->Sequenced();

        if (inst->Results().Length() != 1) {
            continue;
        }

        // Instruction has a single result value.
        // Check to see if the result of this instruction is a candidate for inlining.
        auto* result = inst->Result();
        // Only values with a single usage can be inlined.
        // Named values are not inlined, as we want to emit the name for a let.
        if (result->Usages().Count() == 1 && !ir_->NameOf(result).IsValid()) {
            if (sequenced) {
                // The value comes from a sequenced instruction.  Don't inline.
            } else {
                // The value comes from an unsequenced instruction. Just inline.
                can_inline_.Add(result);
            }
            continue;
        }
    }
}

}  // namespace tint::msl::writer
