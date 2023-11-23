// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/printer/printer.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/ice.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/module.h"
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
#include "src/tint/utils/generator/text_generator.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::msl::writer {
namespace {

/// PIMPL class for the MSL generator
class Printer : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    explicit Printer(core::ir::Module& module) : ir_(module) {}

    /// @returns the generated MSL shader
    tint::Result<std::string> Generate() {
        auto valid = core::ir::ValidateAndDumpIfNeeded(ir_, "MSL writer");
        if (!valid) {
            return std::move(valid.Failure());
        }

        {
            TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
            Line() << "#include <metal_stdlib>";
            Line() << "using namespace metal;";
        }

        // Emit module-scope declarations.
        EmitBlockInstructions(ir_.root_block);

        // Emit functions.
        for (auto& func : ir_.functions) {
            EmitFunction(func);
        }

        StringStream ss;
        ss << preamble_buffer_.String() << std::endl << main_buffer_.String();
        return ss.str();
    }

  private:
    /// Map of builtin structure to unique generated name
    std::unordered_map<const core::type::Struct*, std::string> builtin_struct_names_;

    core::ir::Module& ir_;

    /// The buffer holding preamble text
    TextBuffer preamble_buffer_;

    /// Unique name of the 'TINT_INVARIANT' preprocessor define.
    /// Non-empty only if an invariant attribute has been generated.
    std::string invariant_define_name_;

    std::unordered_set<const core::type::Struct*> emitted_structs_;

    /// The current function being emitted
    core::ir::Function* current_function_ = nullptr;
    /// The current block being emitted
    core::ir::Block* current_block_ = nullptr;

    /// Unique name of the tint_array<T, N> template.
    /// Non-empty only if the template has been generated.
    std::string array_template_name_;

    /// The representation for an IR pointer type
    enum class PtrKind {
        kPtr,  // IR pointer is represented in a pointer
        kRef,  // IR pointer is represented in a reference
    };

    /// The structure for a value held by a 'let', 'var' or parameter.
    struct VariableValue {
        Symbol name;  // Name of the variable
        PtrKind ptr_kind = PtrKind::kRef;
    };

    /// The structure for an inlined value
    struct InlinedValue {
        std::string expr;
        PtrKind ptr_kind = PtrKind::kRef;
    };

    /// Empty struct used as a sentinel value to indicate that an string expression has been
    /// consumed by its single place of usage. Attempting to use this value a second time should
    /// result in an ICE.
    struct ConsumedValue {};

    using ValueBinding = std::variant<VariableValue, InlinedValue, ConsumedValue>;

    /// IR values to their representation
    Hashmap<core::ir::Value*, ValueBinding, 32> bindings_;

    /// Values that can be inlined.
    Hashset<core::ir::Value*, 64> can_inline_;

    /// @returns the name of the templated `tint_array` helper type, generating it if needed
    const std::string& ArrayTemplateName() {
        if (!array_template_name_.empty()) {
            return array_template_name_;
        }

        array_template_name_ = UniqueIdentifier("tint_array");

        TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
        Line() << "template<typename T, size_t N>";
        Line() << "struct " << array_template_name_ << " {";

        {
            ScopedIndent si(current_buffer_);
            Line()
                << "const constant T& operator[](size_t i) const constant { return elements[i]; }";
            for (auto* space : {"device", "thread", "threadgroup"}) {
                Line() << space << " T& operator[](size_t i) " << space
                       << " { return elements[i]; }";
                Line() << "const " << space << " T& operator[](size_t i) const " << space
                       << " { return elements[i]; }";
            }
            Line() << "T elements[N];";
        }
        Line() << "};";
        Line();

        return array_template_name_;
    }

    /// Emit the function
    /// @param func the function to emit
    void EmitFunction(core::ir::Function* func) {
        TINT_SCOPED_ASSIGNMENT(current_function_, func);

        {
            auto out = Line();

            switch (func->Stage()) {
                case core::ir::Function::PipelineStage::kCompute:
                    out << "kernel ";
                    break;
                case core::ir::Function::PipelineStage::kFragment:
                    out << "fragment ";
                    break;
                case core::ir::Function::PipelineStage::kVertex:
                    out << "vertex ";
                    break;
                case core::ir::Function::PipelineStage::kUndefined:
                    break;
            }

            // TODO(dsinclair): Handle return type attributes

            EmitType(out, func->ReturnType());
            out << " " << ir_.NameOf(func).Name() << "() {";

            // TODO(dsinclair): Emit Function parameters
        }
        {
            ScopedIndent si(current_buffer_);
            EmitBlock(func->Block());
        }

        Line() << "}";
    }

    /// Emit a block
    /// @param block the block to emit
    void EmitBlock(core::ir::Block* block) { EmitBlockInstructions(block); }

    /// Emit the instructions in a block
    /// @param block the block with the instructions to emit
    void EmitBlockInstructions(core::ir::Block* block) {
        TINT_SCOPED_ASSIGNMENT(current_block_, block);

        for (auto* inst : *block) {
            Switch(
                inst,                                                //
                [&](core::ir::ExitIf* e) { EmitExitIf(e); },         //
                [&](core::ir::If* if_) { EmitIf(if_); },             //
                [&](core::ir::Return* r) { EmitReturn(r); },         //
                [&](core::ir::Unreachable*) { EmitUnreachable(); },  //
                [&](core::ir::Var* v) { EmitVar(v); },               //
                [&](core::ir::Discard*) { EmitDiscard(); },          //

                [&](core::ir::Bitcast*) { MaybeEmitInstruction(inst); },    //
                [&](core::ir::Binary*) { MaybeEmitInstruction(inst); },     //
                [&](core::ir::Let* l) { EmitLet(l); },                      //
                [&](core::ir::Load*) { MaybeEmitInstruction(inst); },       //
                [&](core::ir::Construct*) { MaybeEmitInstruction(inst); },  //
                [&](core::ir::Access*) { MaybeEmitInstruction(inst); },     //
                TINT_ICE_ON_NO_MATCH);
        }
    }

    // If the instruction is named, we need to emit it. If it is un-named, then we'll use it
    // and inline it later.
    void MaybeEmitInstruction(const core::ir::Instruction* inst) {
        auto name = ir_.NameOf(inst->Result());
        if (!name.IsValid()) {
            return;
        }

        auto out = Line();
        EmitType(out, inst->Result()->Type());
        out << " const " << name.Name() << " = ";
        EmitValue(out, inst->Result());
        out << ";";
    }

    void EmitValue(StringStream& out, const core::ir::Value* v) {
        Switch(
            v,                                                           //
            [&](const core::ir::Constant* c) { EmitConstant(out, c); },  //
            // [&](core::ir::FunctionParam* fp) {},                   //
            [&](const core::ir::InstructionResult* r) {
                Switch(
                    r->Source(),                                             //
                    [&](const core::ir::Binary* b) { EmitBinary(out, b); },  //
                    [&](const core::ir::Let* l) {
                        auto name = ir_.NameOf(l->Result());
                        TINT_ASSERT(name.IsValid());
                        out << name.Name();
                    },                                                             //
                    [&](const core::ir::Load* l) { EmitValue(out, l->From()); },   //
                    [&](const core::ir::Construct* c) { EmitConstruct(out, c); },  //
                    [&](const core::ir::Var* var) { EmitVarName(out, var); },      //
                    [&](const core::ir::Bitcast* b) { EmitBitcast(out, b); },      //
                    [&](const core::ir::Access* a) { EmitAccess(out, a); },        //
                    TINT_ICE_ON_NO_MATCH);
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Emit a binary instruction
    /// @param b the binary instruction
    void EmitBinary(StringStream& out, const core::ir::Binary* b) {
        if (b->Op() == core::ir::BinaryOp::kEqual) {
            auto* rhs = b->RHS()->As<core::ir::Constant>();
            if (rhs && rhs->Type()->Is<core::type::Bool>() &&
                rhs->Value()->ValueAs<bool>() == false) {
                // expr == false
                out << "!(";
                EmitValue(out, b->LHS());
                out << ")";
                return;
            }
        }

        auto kind = [&] {
            switch (b->Op()) {
                case core::ir::BinaryOp::kAdd:
                    return "+";
                case core::ir::BinaryOp::kSubtract:
                    return "-";
                case core::ir::BinaryOp::kMultiply:
                    return "*";
                case core::ir::BinaryOp::kDivide:
                    return "/";
                case core::ir::BinaryOp::kModulo:
                    return "%";
                case core::ir::BinaryOp::kAnd:
                    return "&";
                case core::ir::BinaryOp::kOr:
                    return "|";
                case core::ir::BinaryOp::kXor:
                    return "^";
                case core::ir::BinaryOp::kEqual:
                    return "==";
                case core::ir::BinaryOp::kNotEqual:
                    return "!=";
                case core::ir::BinaryOp::kLessThan:
                    return "<";
                case core::ir::BinaryOp::kGreaterThan:
                    return ">";
                case core::ir::BinaryOp::kLessThanEqual:
                    return "<=";
                case core::ir::BinaryOp::kGreaterThanEqual:
                    return ">=";
                case core::ir::BinaryOp::kShiftLeft:
                    return "<<";
                case core::ir::BinaryOp::kShiftRight:
                    return ">>";
            }
            return "<error>";
        };

        out << "(";
        EmitValue(out, b->LHS());
        out << " " << kind() << " ";
        EmitValue(out, b->RHS());
        out << ")";
    }

    void EmitVarName(StringStream& out, const core::ir::Var* v) { out << ir_.NameOf(v).Name(); }

    /// Emit a var instruction
    /// @param v the var instruction
    void EmitVar(core::ir::Var* v) {
        auto out = Line();

        auto* ptr = v->Result()->Type()->As<core::type::Pointer>();
        TINT_ASSERT_OR_RETURN(ptr);

        auto space = ptr->AddressSpace();
        switch (space) {
            case core::AddressSpace::kFunction:
            case core::AddressSpace::kHandle:
                break;
            case core::AddressSpace::kPrivate:
                out << "thread ";
                break;
            case core::AddressSpace::kWorkgroup:
                out << "threadgroup ";
                break;
            default:
                TINT_IR_ICE(ir_) << "unhandled variable address space";
                return;
        }

        auto name = ir_.NameOf(v);

        EmitType(out, ptr->UnwrapPtr());
        out << " " << name.Name();

        if (v->Initializer()) {
            out << " = ";
            EmitValue(out, v->Initializer());
        } else if (space == core::AddressSpace::kPrivate ||
                   space == core::AddressSpace::kFunction ||
                   space == core::AddressSpace::kUndefined) {
            out << " = ";
            EmitZeroValue(out, ptr->UnwrapPtr());
        }
        out << ";";
    }

    /// Emit a let instruction
    /// @param l the let instruction
    void EmitLet(core::ir::Let* l) {
        auto name = ir_.NameOf(l->Result());
        TINT_ASSERT(name.IsValid());

        auto out = Line();
        EmitType(out, l->Result()->Type());
        out << " const " << name.Name() << " = ";
        EmitValue(out, l->Value());
        out << ";";
    }

    /// Emit an if instruction
    /// @param if_ the if instruction
    void EmitIf(core::ir::If* if_) {
        {
            auto out = Line();
            out << "if (";
            EmitValue(out, if_->Condition());
            out << ") {";
        }

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

    /// Emit an exit-if instruction
    /// @param e the exit-if instruction
    void EmitExitIf(core::ir::ExitIf* e) {
        auto results = e->If()->Results();
        auto args = e->Args();
        for (size_t i = 0; i < e->Args().Length(); ++i) {
            auto* phi = results[i];
            auto* val = args[i];

            auto out = Line();
            out << ir_.NameOf(phi).Name() << " = ";
            EmitValue(out, val);
            out << ";";
        }
    }

    /// Emit a return instruction
    /// @param r the return instruction
    void EmitReturn(core::ir::Return* r) {
        // If this return has no arguments and the current block is for the function which is
        // being returned, skip the return.
        if (current_block_ == current_function_->Block() && r->Args().IsEmpty()) {
            return;
        }

        auto out = Line();
        out << "return";
        if (!r->Args().IsEmpty()) {
            out << " ";
            EmitValue(out, r->Args().Front());
        }
        out << ";";
    }

    /// Emit an unreachable instruction
    void EmitUnreachable() { Line() << "/* unreachable */"; }

    /// Emit a discard instruction
    void EmitDiscard() { Line() << "discard_fragment();"; }

    /// Emit a bitcast instruction
    void EmitBitcast(StringStream& out, const core::ir::Bitcast* b) {
        out << "as_type<";
        EmitType(out, b->Result()->Type());
        out << ">(";
        EmitValue(out, b->Val());
        out << ")";
    }

    /// Emit an accessor
    void EmitAccess(StringStream& out, const core::ir::Access* a) {
        EmitValue(out, a->Object());

        auto* current_type = a->Result()->Type();
        for (auto* index : a->Indices()) {
            Switch(
                current_type,  //
                [&](const core::type::Struct* s) {
                    auto* c = index->As<core::ir::Constant>();
                    auto* member = s->Members()[c->Value()->ValueAs<uint32_t>()];
                    out << "." << member->Name().Name();
                    current_type = member->Type();
                },
                [&](Default) {
                    out << "[";
                    EmitValue(out, index);
                    out << "]";
                });
        }
    }

    /// Emit a constructor
    void EmitConstruct(StringStream& out, const core::ir::Construct* c) {
        Switch(
            c->Result()->Type(),
            [&](const core::type::Array*) {
                EmitType(out, c->Result()->Type());
                out << "{";
                size_t i = 0;
                for (auto* arg : c->Args()) {
                    if (i > 0) {
                        out << ", ";
                    }
                    EmitValue(out, arg);
                    i++;
                }
                out << "}";
            },
            [&](const core::type::Struct* struct_ty) {
                out << "{";
                size_t i = 0;
                for (auto* arg : c->Args()) {
                    if (i > 0) {
                        out << ", ";
                    }
                    // Emit field designators for structures to account for padding members.
                    auto name = struct_ty->Members()[i]->Name().Name();
                    out << "." << name << "=";
                    EmitValue(out, arg);
                    i++;
                }
                out << "}";
            },
            [&](Default) {
                EmitType(out, c->Result()->Type());
                out << "(";
                size_t i = 0;
                for (auto* arg : c->Args()) {
                    if (i > 0) {
                        out << ", ";
                    }
                    EmitValue(out, arg);
                    i++;
                }
                out << ")";
            });
    }

    /// Handles generating a address space
    /// @param out the output of the type stream
    /// @param sc the address space to generate
    void EmitAddressSpace(StringStream& out, core::AddressSpace sc) {
        switch (sc) {
            case core::AddressSpace::kFunction:
            case core::AddressSpace::kPrivate:
            case core::AddressSpace::kHandle:
                out << "thread";
                break;
            case core::AddressSpace::kWorkgroup:
                out << "threadgroup";
                break;
            case core::AddressSpace::kStorage:
                out << "device";
                break;
            case core::AddressSpace::kUniform:
                out << "constant";
                break;
            default:
                TINT_IR_ICE(ir_) << "unhandled address space: " << sc;
                break;
        }
    }

    /// Emit a type
    /// @param out the stream to emit too
    /// @param ty the type to emit
    void EmitType(StringStream& out, const core::type::Type* ty) {
        tint::Switch(
            ty,                                               //
            [&](const core::type::Bool*) { out << "bool"; },  //
            [&](const core::type::Void*) { out << "void"; },  //
            [&](const core::type::F32*) { out << "float"; },  //
            [&](const core::type::F16*) { out << "half"; },   //
            [&](const core::type::I32*) { out << "int"; },    //
            [&](const core::type::U32*) { out << "uint"; },   //
            [&](const core::type::Array* arr) { EmitArrayType(out, arr); },
            [&](const core::type::Vector* vec) { EmitVectorType(out, vec); },
            [&](const core::type::Matrix* mat) { EmitMatrixType(out, mat); },
            [&](const core::type::Atomic* atomic) { EmitAtomicType(out, atomic); },
            [&](const core::type::Pointer* ptr) { EmitPointerType(out, ptr); },
            [&](const core::type::Sampler*) { out << "sampler"; },  //
            [&](const core::type::Texture* tex) { EmitTextureType(out, tex); },
            [&](const core::type::Struct* str) {
                out << StructName(str);

                TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
                EmitStructType(str);
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Handles generating a pointer declaration
    /// @param out the output stream
    /// @param ptr the pointer to emit
    void EmitPointerType(StringStream& out, const core::type::Pointer* ptr) {
        if (ptr->Access() == core::Access::kRead) {
            out << "const ";
        }
        EmitAddressSpace(out, ptr->AddressSpace());
        out << " ";
        EmitType(out, ptr->StoreType());
        out << "*";
    }

    /// Handles generating an atomic declaration
    /// @param out the output stream
    /// @param atomic the atomic to emit
    void EmitAtomicType(StringStream& out, const core::type::Atomic* atomic) {
        if (atomic->Type()->Is<core::type::I32>()) {
            out << "atomic_int";
            return;
        }
        if (TINT_LIKELY(atomic->Type()->Is<core::type::U32>())) {
            out << "atomic_uint";
            return;
        }
        TINT_ICE() << "unhandled atomic type " << atomic->Type()->FriendlyName();
    }

    /// Handles generating an array declaration
    /// @param out the output stream
    /// @param arr the array to emit
    void EmitArrayType(StringStream& out, const core::type::Array* arr) {
        out << ArrayTemplateName() << "<";
        EmitType(out, arr->ElemType());
        out << ", ";
        if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
            out << "1";
        } else {
            auto count = arr->ConstantCount();
            if (!count) {
                TINT_IR_ICE(ir_) << core::type::Array::kErrExpectedConstantCount;
                return;
            }
            out << count.value();
        }
        out << ">";
    }

    /// Handles generating a vector declaration
    /// @param out the output stream
    /// @param vec the vector to emit
    void EmitVectorType(StringStream& out, const core::type::Vector* vec) {
        if (vec->Packed()) {
            out << "packed_";
        }
        EmitType(out, vec->type());
        out << vec->Width();
    }

    /// Handles generating a matrix declaration
    /// @param out the output stream
    /// @param mat the matrix to emit
    void EmitMatrixType(StringStream& out, const core::type::Matrix* mat) {
        EmitType(out, mat->type());
        out << mat->columns() << "x" << mat->rows();
    }

    /// Handles generating a texture declaration
    /// @param out the output stream
    /// @param tex the texture to emit
    void EmitTextureType(StringStream& out, const core::type::Texture* tex) {
        if (TINT_UNLIKELY(tex->Is<core::type::ExternalTexture>())) {
            TINT_IR_ICE(ir_) << "Multiplanar external texture transform was not run.";
            return;
        }

        if (tex->IsAnyOf<core::type::DepthTexture, core::type::DepthMultisampledTexture>()) {
            out << "depth";
        } else {
            out << "texture";
        }

        switch (tex->dim()) {
            case core::type::TextureDimension::k1d:
                out << "1d";
                break;
            case core::type::TextureDimension::k2d:
                out << "2d";
                break;
            case core::type::TextureDimension::k2dArray:
                out << "2d_array";
                break;
            case core::type::TextureDimension::k3d:
                out << "3d";
                break;
            case core::type::TextureDimension::kCube:
                out << "cube";
                break;
            case core::type::TextureDimension::kCubeArray:
                out << "cube_array";
                break;
            default:
                TINT_IR_ICE(ir_) << "invalid texture dimensions";
                return;
        }
        if (tex->IsAnyOf<core::type::MultisampledTexture, core::type::DepthMultisampledTexture>()) {
            out << "_ms";
        }
        out << "<";
        TINT_DEFER(out << ">");

        tint::Switch(
            tex,  //
            [&](const core::type::DepthTexture*) { out << "float, access::sample"; },
            [&](const core::type::DepthMultisampledTexture*) { out << "float, access::read"; },
            [&](const core::type::StorageTexture* storage) {
                EmitType(out, storage->type());
                out << ", ";

                std::string access_str;
                if (storage->access() == core::Access::kRead) {
                    out << "access::read";
                } else if (storage->access() == core::Access::kWrite) {
                    out << "access::write";
                } else {
                    TINT_IR_ICE(ir_) << "invalid access control for storage texture";
                    return;
                }
            },
            [&](const core::type::MultisampledTexture* ms) {
                EmitType(out, ms->type());
                out << ", access::read";
            },
            [&](const core::type::SampledTexture* sampled) {
                EmitType(out, sampled->type());
                out << ", access::sample";
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Handles generating a struct declaration. If the structure has already been emitted, then
    /// this function will simply return without emitting anything.
    /// @param str the struct to generate
    void EmitStructType(const core::type::Struct* str) {
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
            } while (str->FindMember(ir_.symbols.Get(name)));

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
                    TINT_IR_ICE(ir_) << "Structure member offset (" << ir_offset
                                     << ") is behind MSL offset (" << msl_offset << ")";
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
                    TINT_IR_ICE(ir_) << "unknown builtin";
                    return;
                }
                out << " [[" << name << "]]";
            }

            if (auto location = attributes.location) {
                auto& pipeline_stage_uses = str->PipelineStageUses();
                if (TINT_UNLIKELY(pipeline_stage_uses.size() != 1)) {
                    TINT_IR_ICE(ir_) << "invalid entry point IO struct uses";
                    return;
                }

                if (pipeline_stage_uses.count(core::type::PipelineStageUsage::kVertexInput)) {
                    out << " [[attribute(" + std::to_string(location.value()) + ")]]";
                } else if (pipeline_stage_uses.count(
                               core::type::PipelineStageUsage::kVertexOutput)) {
                    out << " [[user(locn" + std::to_string(location.value()) + ")]]";
                } else if (pipeline_stage_uses.count(
                               core::type::PipelineStageUsage::kFragmentInput)) {
                    out << " [[user(locn" + std::to_string(location.value()) + ")]]";
                } else if (TINT_LIKELY(pipeline_stage_uses.count(
                               core::type::PipelineStageUsage::kFragmentOutput))) {
                    out << " [[color(" + std::to_string(location.value()) + ")]]";
                } else {
                    TINT_IR_ICE(ir_) << "invalid use of location decoration";
                    return;
                }
            }

            if (auto interpolation = attributes.interpolation) {
                auto name = InterpolationToAttribute(interpolation->type, interpolation->sampling);
                if (name.empty()) {
                    TINT_IR_ICE(ir_) << "unknown interpolation attribute";
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
                auto size_align = MslPackedTypeSizeAndAlign(ty);
                if (TINT_UNLIKELY(msl_offset % size_align.align)) {
                    TINT_IR_ICE(ir_) << "Misaligned MSL structure member " << mem_name << " : "
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

    /// Handles core::ir::Constant values
    /// @param out the stream to write the constant too
    /// @param c the constant to emit
    void EmitConstant(StringStream& out, const core::ir::Constant* c) {
        EmitConstant(out, c->Value());
    }

    /// Handles core::constant::Value values
    /// @param out the stream to write the constant too
    /// @param c the constant to emit
    void EmitConstant(StringStream& out, const core::constant::Value* c) {
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
            [&](const core::type::Bool*) { out << (c->ValueAs<bool>() ? "true" : "false"); },
            [&](const core::type::I32*) { PrintI32(out, c->ValueAs<i32>()); },
            [&](const core::type::U32*) { out << c->ValueAs<u32>() << "u"; },
            [&](const core::type::F32*) { PrintF32(out, c->ValueAs<f32>()); },
            [&](const core::type::F16*) { PrintF16(out, c->ValueAs<f16>()); },
            [&](const core::type::Vector* v) {
                EmitType(out, v);

                ScopedParen sp(out);
                if (auto* splat = c->As<core::constant::Splat>()) {
                    EmitConstant(out, splat->el);
                    return;
                }
                emit_values(v->Width());
            },
            [&](const core::type::Matrix* m) {
                EmitType(out, m);
                ScopedParen sp(out);
                emit_values(m->columns());
            },
            [&](const core::type::Array* a) {
                EmitType(out, a);
                out << "{";
                TINT_DEFER(out << "}");

                if (c->AllZero()) {
                    return;
                }

                auto count = a->ConstantCount();
                if (!count) {
                    TINT_IR_ICE(ir_) << core::type::Array::kErrExpectedConstantCount;
                    return;
                }
                emit_values(*count);
            },
            [&](const core::type::Struct* s) {
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
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Emits the zero value for the given type
    /// @param out the stream to emit too
    /// @param ty the type
    void EmitZeroValue(StringStream& out, const core::type::Type* ty) {
        Switch(
            ty, [&](const core::type::Bool*) { out << "false"; },                     //
            [&](const core::type::F16*) { out << "0.0h"; },                           //
            [&](const core::type::F32*) { out << "0.0f"; },                           //
            [&](const core::type::I32*) { out << "0"; },                              //
            [&](const core::type::U32*) { out << "0u"; },                             //
            [&](const core::type::Vector* vec) { EmitZeroValue(out, vec->type()); },  //
            [&](const core::type::Matrix* mat) {
                EmitType(out, mat);

                ScopedParen sp(out);
                EmitZeroValue(out, mat->type());
            },
            [&](const core::type::Array*) { out << "{}"; },   //
            [&](const core::type::Struct*) { out << "{}"; },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// @param s the structure
    /// @returns the name of the structure, taking special care of builtin structures that start
    /// with double underscores. If the structure is a builtin, then the returned name will be a
    /// unique name without the leading underscores.
    std::string StructName(const core::type::Struct* s) {
        auto name = s->Name().Name();
        if (HasPrefix(name, "__")) {
            name = tint::GetOrCreate(builtin_struct_names_, s,
                                     [&] { return UniqueIdentifier(name.substr(2)); });
        }
        return name;
    }

    /// @return a new, unique identifier with the given prefix.
    /// @param prefix optional prefix to apply to the generated identifier. If empty
    /// "tint_symbol" will be used.
    std::string UniqueIdentifier(const std::string& prefix /* = "" */) {
        return ir_.symbols.New(prefix).Name();
    }
};

}  // namespace

Result<std::string> Print(core::ir::Module& module) {
    return Printer{module}.Generate();
}

}  // namespace tint::msl::writer
