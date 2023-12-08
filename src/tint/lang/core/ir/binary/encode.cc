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

#include "src/tint/lang/core/ir/binary/encode.h"

#include <utility>

#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/rtti/switch.h"

TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS();
#include "src/tint/lang/core/ir/binary/ir.pb.h"
TINT_END_DISABLE_PROTOBUF_WARNINGS();

namespace tint::core::ir::binary {
namespace {
struct Encoder {
    const Module& mod_in_;
    pb::Module& mod_out_;
    Hashmap<const core::ir::Function*, uint32_t, 32> functions_{};
    Hashmap<const core::ir::Block*, uint32_t, 32> blocks_{};
    Hashmap<const core::type::Type*, uint32_t, 32> types_{};
    Hashmap<const core::ir::Value*, uint32_t, 32> values_{};
    Hashmap<const core::constant::Value*, uint32_t, 32> constant_values_{};

    void Encode() {
        Vector<pb::Function*, 8> fns_out;
        for (auto& fn_in : mod_in_.functions) {
            uint32_t id = static_cast<uint32_t>(fns_out.Length() + 1);
            fns_out.Push(mod_out_.add_functions());
            functions_.Add(fn_in, id);
        }
        for (size_t i = 0, n = mod_in_.functions.Length(); i < n; i++) {
            PopulateFunction(fns_out[i], mod_in_.functions[i]);
        }
        mod_out_.set_root_block(Block(mod_in_.root_block));
    }

    ////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////
    void PopulateFunction(pb::Function* fn_out, const ir::Function* fn_in) {
        if (auto name = mod_in_.NameOf(fn_in)) {
            fn_out->set_name(name.Name());
        }
        fn_out->set_return_type(Type(fn_in->ReturnType()));
        if (fn_in->Stage() != Function::PipelineStage::kUndefined) {
            fn_out->set_pipeline_stage(PipelineStage(fn_in->Stage()));
        }
        if (auto wg_size_in = fn_in->WorkgroupSize()) {
            auto& wg_size_out = *fn_out->mutable_workgroup_size();
            wg_size_out.set_x((*wg_size_in)[0]);
            wg_size_out.set_y((*wg_size_in)[1]);
            wg_size_out.set_z((*wg_size_in)[2]);
        }
        for (auto* param_in : fn_in->Params()) {
            fn_out->add_parameters(Value(param_in));
        }
        fn_out->set_block(Block(fn_in->Block()));
    }

    uint32_t Function(const ir::Function* fn_in) { return fn_in ? *functions_.Get(fn_in) : 0; }

    pb::PipelineStage PipelineStage(Function::PipelineStage stage) {
        switch (stage) {
            case Function::PipelineStage::kCompute:
                return pb::PipelineStage::Compute;
            case Function::PipelineStage::kFragment:
                return pb::PipelineStage::Fragment;
            case Function::PipelineStage::kVertex:
                return pb::PipelineStage::Vertex;
            default:
                TINT_ICE() << "unhandled PipelineStage: " << stage;
                return pb::PipelineStage::Compute;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Blocks
    ////////////////////////////////////////////////////////////////////////////
    uint32_t Block(const ir::Block* block_in) {
        if (block_in == nullptr) {
            return 0;
        }
        return blocks_.GetOrCreate(block_in, [&]() -> uint32_t {
            auto& block_out = *mod_out_.add_blocks();
            for (auto* inst : *block_in) {
                Instruction(*block_out.add_instructions(), inst);
            }
            return static_cast<uint32_t>(blocks_.Count());
        });
    }

    ////////////////////////////////////////////////////////////////////////////
    // Instructions
    ////////////////////////////////////////////////////////////////////////////
    void Instruction(pb::Instruction& inst_out, const ir::Instruction* inst_in) {
        Switch(
            inst_in,  //
            [&](const ir::Access* i) { InstructionAccess(*inst_out.mutable_access(), i); },
            [&](const ir::Binary* i) { InstructionBinary(*inst_out.mutable_binary(), i); },
            [&](const ir::Construct* i) { InstructionConstruct(*inst_out.mutable_construct(), i); },
            [&](const ir::Discard* i) { InstructionDiscard(*inst_out.mutable_discard(), i); },
            [&](const ir::Let* i) { InstructionLet(*inst_out.mutable_let(), i); },
            [&](const ir::Load* i) { InstructionLoad(*inst_out.mutable_load(), i); },
            [&](const ir::LoadVectorElement* i) {
                InstructionLoadVectorElement(*inst_out.mutable_load_vector_element(), i);
            },
            [&](const ir::Return* i) { InstructionReturn(*inst_out.mutable_return_(), i); },
            [&](const ir::Store* i) { InstructionStore(*inst_out.mutable_store(), i); },
            [&](const ir::StoreVectorElement* i) {
                InstructionStoreVectorElement(*inst_out.mutable_store_vector_element(), i);
            },
            [&](const ir::Swizzle* i) { InstructionSwizzle(*inst_out.mutable_swizzle(), i); },
            [&](const ir::Unary* i) { InstructionUnary(*inst_out.mutable_unary(), i); },
            [&](const ir::UserCall* i) { InstructionUserCall(*inst_out.mutable_user_call(), i); },
            [&](const ir::Var* i) { InstructionVar(*inst_out.mutable_var(), i); },
            TINT_ICE_ON_NO_MATCH);
        for (auto* operand : inst_in->Operands()) {
            inst_out.add_operands(Value(operand));
        }
        for (auto* result : inst_in->Results()) {
            inst_out.add_results(Value(result));
        }
    }

    void InstructionAccess(pb::InstructionAccess&, const ir::Access*) {}

    void InstructionBinary(pb::InstructionBinary& binary_out, const ir::Binary* binary_in) {
        binary_out.set_op(BinaryOp(binary_in->Op()));
    }

    void InstructionConstruct(pb::InstructionConstruct&, const ir::Construct*) {}

    void InstructionDiscard(pb::InstructionDiscard&, const ir::Discard*) {}

    void InstructionLet(pb::InstructionLet&, const ir::Let*) {}

    void InstructionLoad(pb::InstructionLoad&, const ir::Load*) {}

    void InstructionLoadVectorElement(pb::InstructionLoadVectorElement&,
                                      const ir::LoadVectorElement*) {}

    void InstructionReturn(pb::InstructionReturn&, const ir::Return*) {}

    void InstructionStore(pb::InstructionStore&, const ir::Store*) {}

    void InstructionStoreVectorElement(pb::InstructionStoreVectorElement&,
                                       const ir::StoreVectorElement*) {}

    void InstructionSwizzle(pb::InstructionSwizzle& swizzle_out, const ir::Swizzle* swizzle_in) {
        for (auto idx : swizzle_in->Indices()) {
            swizzle_out.add_indices(idx);
        }
    }

    void InstructionUnary(pb::InstructionUnary& unary_out, const ir::Unary* unary_in) {
        unary_out.set_op(UnaryOp(unary_in->Op()));
    }

    void InstructionUserCall(pb::InstructionUserCall&, const ir::UserCall*) {}

    void InstructionVar(pb::InstructionVar& var_out, const ir::Var* var_in) {
        if (auto bp_in = var_in->BindingPoint()) {
            auto& bp_out = *var_out.mutable_binding_point();
            bp_out.set_group(bp_in->group);
            bp_out.set_binding(bp_in->binding);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////
    uint32_t Type(const core::type::Type* type_in) {
        if (type_in == nullptr) {
            return 0;
        }
        return types_.GetOrCreate(type_in, [&]() -> uint32_t {
            pb::Type type_out;
            Switch(
                type_in,  //
                [&](const core::type::Void*) { type_out.set_basic(pb::TypeBasic::void_); },
                [&](const core::type::Bool*) { type_out.set_basic(pb::TypeBasic::bool_); },
                [&](const core::type::I32*) { type_out.set_basic(pb::TypeBasic::i32); },
                [&](const core::type::U32*) { type_out.set_basic(pb::TypeBasic::u32); },
                [&](const core::type::F32*) { type_out.set_basic(pb::TypeBasic::f32); },
                [&](const core::type::F16*) { type_out.set_basic(pb::TypeBasic::f16); },
                [&](const core::type::Vector* v) { TypeVector(*type_out.mutable_vector(), v); },
                [&](const core::type::Matrix* m) { TypeMatrix(*type_out.mutable_matrix(), m); },
                [&](const core::type::Pointer* m) { TypePointer(*type_out.mutable_pointer(), m); },
                [&](const core::type::Array* m) { TypeArray(*type_out.mutable_array(), m); },
                TINT_ICE_ON_NO_MATCH);

            mod_out_.mutable_types()->Add(std::move(type_out));
            return static_cast<uint32_t>(mod_out_.types().size());
        });
    }

    void TypeVector(pb::TypeVector& vector_out, const core::type::Vector* vector_in) {
        vector_out.set_width(vector_in->Width());
        vector_out.set_element_type(Type(vector_in->type()));
    }

    void TypeMatrix(pb::TypeMatrix& matrix_out, const core::type::Matrix* matrix_in) {
        matrix_out.set_num_columns(matrix_in->columns());
        matrix_out.set_num_rows(matrix_in->rows());
        matrix_out.set_element_type(Type(matrix_in->type()));
    }

    void TypePointer(pb::TypePointer& pointer_out, const core::type::Pointer* pointer_in) {
        pointer_out.set_address_space(AddressSpace(pointer_in->AddressSpace()));
        pointer_out.set_store_type(Type(pointer_in->StoreType()));
        pointer_out.set_access(Access(pointer_in->Access()));
    }

    void TypeArray(pb::TypeArray& array_out, const core::type::Array* array_in) {
        array_out.set_element(Type(array_in->ElemType()));
        array_out.set_stride(array_in->Stride());
        Switch(
            array_in->Count(),  //
            [&](const core::type::ConstantArrayCount* c) { array_out.set_count(c->value); },
            TINT_ICE_ON_NO_MATCH);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Values
    ////////////////////////////////////////////////////////////////////////////
    uint32_t Value(const ir::Value* value_in) {
        if (!value_in) {
            return 0;
        }
        return values_.GetOrCreate(value_in, [&] {
            auto& value_out = *mod_out_.add_values();
            Switch(
                value_in,
                [&](const ir::InstructionResult* v) {
                    InstructionResult(*value_out.mutable_instruction_result(), v);
                },
                [&](const ir::FunctionParam* v) {
                    FunctionParameter(*value_out.mutable_function_parameter(), v);
                },
                [&](const ir::Function* v) { value_out.set_function(Function(v)); },
                [&](const ir::Constant* v) { value_out.set_constant(ConstantValue(v->Value())); },
                TINT_ICE_ON_NO_MATCH);

            return static_cast<uint32_t>(mod_out_.values().size());
        });
    }

    void InstructionResult(pb::InstructionResult& res_out, const ir::InstructionResult* res_in) {
        res_out.set_type(Type(res_in->Type()));
        if (auto name = mod_in_.NameOf(res_in); name.IsValid()) {
            res_out.set_name(name.Name());
        }
    }

    void FunctionParameter(pb::FunctionParameter& param_out, const ir::FunctionParam* param_in) {
        param_out.set_type(Type(param_in->Type()));
        if (auto name = mod_in_.NameOf(param_in); name.IsValid()) {
            param_out.set_name(name.Name());
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // ConstantValues
    ////////////////////////////////////////////////////////////////////////////
    uint32_t ConstantValue(const core::constant::Value* constant_in) {
        if (!constant_in) {
            return 0;
        }
        return constant_values_.GetOrCreate(constant_in, [&] {
            pb::ConstantValue constant_out;
            Switch(
                constant_in,  //
                [&](const core::constant::Scalar<bool>* b) {
                    constant_out.mutable_scalar()->set_bool_(b->value);
                },
                [&](const core::constant::Scalar<core::i32>* i32) {
                    constant_out.mutable_scalar()->set_i32(i32->value);
                },
                [&](const core::constant::Scalar<core::u32>* u32) {
                    constant_out.mutable_scalar()->set_u32(u32->value);
                },
                [&](const core::constant::Scalar<core::f32>* f32) {
                    constant_out.mutable_scalar()->set_f32(f32->value);
                },
                [&](const core::constant::Scalar<core::f16>* f16) {
                    constant_out.mutable_scalar()->set_f16(f16->value);
                },
                [&](const core::constant::Composite* composite) {
                    ConstantValueComposite(*constant_out.mutable_composite(), composite);
                },
                [&](const core::constant::Splat* splat) {
                    ConstantValueSplat(*constant_out.mutable_splat(), splat);
                },
                TINT_ICE_ON_NO_MATCH);

            mod_out_.mutable_constant_values()->Add(std::move(constant_out));
            return static_cast<uint32_t>(mod_out_.constant_values().size());
        });
    }

    void ConstantValueComposite(pb::ConstantValueComposite& composite_out,
                                const core::constant::Composite* composite_in) {
        composite_out.set_type(Type(composite_in->type));
        for (auto* el : composite_in->elements) {
            composite_out.add_elements(ConstantValue(el));
        }
    }

    void ConstantValueSplat(pb::ConstantValueSplat& splat_out,
                            const core::constant::Splat* splat_in) {
        splat_out.set_type(Type(splat_in->type));
        splat_out.set_elements(ConstantValue(splat_in->el));
        splat_out.set_count(static_cast<uint32_t>(splat_in->count));
    }

    ////////////////////////////////////////////////////////////////////////////
    // Enums
    ////////////////////////////////////////////////////////////////////////////
    pb::AddressSpace AddressSpace(core::AddressSpace in) {
        switch (in) {
            case core::AddressSpace::kFunction:
                return pb::AddressSpace::function;
            case core::AddressSpace::kHandle:
                return pb::AddressSpace::handle;
            case core::AddressSpace::kPixelLocal:
                return pb::AddressSpace::pixel_local;
            case core::AddressSpace::kPrivate:
                return pb::AddressSpace::private_;
            case core::AddressSpace::kPushConstant:
                return pb::AddressSpace::push_constant;
            case core::AddressSpace::kStorage:
                return pb::AddressSpace::storage;
            case core::AddressSpace::kUniform:
                return pb::AddressSpace::uniform;
            case core::AddressSpace::kWorkgroup:
                return pb::AddressSpace::workgroup;
            default:
                TINT_ICE() << "invalid AddressSpace: " << in;
                return pb::AddressSpace::function;
        }
    }

    pb::AccessControl Access(core::Access in) {
        switch (in) {
            case core::Access::kRead:
                return pb::AccessControl::read;
            case core::Access::kWrite:
                return pb::AccessControl::write;
            case core::Access::kReadWrite:
                return pb::AccessControl::read_write;
            default:
                TINT_ICE() << "invalid Access: " << in;
                return pb::AccessControl::read;
        }
    }

    pb::UnaryOp UnaryOp(core::ir::UnaryOp in) {
        switch (in) {
            case core::ir::UnaryOp::kComplement:
                return pb::UnaryOp::complement;
            case core::ir::UnaryOp::kNegation:
                return pb::UnaryOp::negation;
        }
        TINT_ICE() << "invalid UnaryOp: " << in;
        return pb::UnaryOp::complement;
    }

    pb::BinaryOp BinaryOp(core::ir::BinaryOp in) {
        switch (in) {
            case core::ir::BinaryOp::kAdd:
                return pb::BinaryOp::add_;
            case core::ir::BinaryOp::kSubtract:
                return pb::BinaryOp::subtract;
            case core::ir::BinaryOp::kMultiply:
                return pb::BinaryOp::multiply;
            case core::ir::BinaryOp::kDivide:
                return pb::BinaryOp::divide;
            case core::ir::BinaryOp::kModulo:
                return pb::BinaryOp::modulo;
            case core::ir::BinaryOp::kAnd:
                return pb::BinaryOp::and_;
            case core::ir::BinaryOp::kOr:
                return pb::BinaryOp::or_;
            case core::ir::BinaryOp::kXor:
                return pb::BinaryOp::xor_;
            case core::ir::BinaryOp::kEqual:
                return pb::BinaryOp::equal;
            case core::ir::BinaryOp::kNotEqual:
                return pb::BinaryOp::not_equal;
            case core::ir::BinaryOp::kLessThan:
                return pb::BinaryOp::less_than;
            case core::ir::BinaryOp::kGreaterThan:
                return pb::BinaryOp::greater_than;
            case core::ir::BinaryOp::kLessThanEqual:
                return pb::BinaryOp::less_than_equal;
            case core::ir::BinaryOp::kGreaterThanEqual:
                return pb::BinaryOp::greater_than_equal;
            case core::ir::BinaryOp::kShiftLeft:
                return pb::BinaryOp::shift_left;
            case core::ir::BinaryOp::kShiftRight:
                return pb::BinaryOp::shift_right;
        }

        TINT_ICE() << "invalid BinaryOp: " << in;
        return pb::BinaryOp::add_;
    }
};

}  // namespace

Result<Vector<std::byte, 0>> Encode(const Module& mod_in) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    pb::Module mod_out;
    Encoder{mod_in, mod_out}.Encode();

    Vector<std::byte, 0> buffer;
    size_t len = mod_out.ByteSizeLong();
    buffer.Resize(len);
    if (len > 0) {
        if (!mod_out.SerializeToArray(&buffer[0], static_cast<int>(len))) {
            return Failure{"failed to serialize protobuf"};
        }
    }
    return buffer;
}

}  // namespace tint::core::ir::binary
