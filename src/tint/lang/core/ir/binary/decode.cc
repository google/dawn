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

#include "src/tint/lang/core/ir/binary/decode.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/macros/compiler.h"

TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS();
#include "src/tint/lang/core/ir/binary/ir.pb.h"
TINT_END_DISABLE_PROTOBUF_WARNINGS();

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir::binary {
namespace {

struct Decoder {
    pb::Module& mod_in_;
    Module& mod_out_;
    Vector<ir::Block*, 32> blocks_{};
    Vector<const type::Type*, 32> types_{};
    Vector<const core::constant::Value*, 32> constant_values_{};
    Vector<ir::Value*, 32> values_{};
    Builder b{mod_out_};

    void Decode() {
        {
            const size_t n = static_cast<size_t>(mod_in_.types().size());
            types_.Reserve(n);
            for (auto& type_in : mod_in_.types()) {
                types_.Push(CreateType(type_in));
            }
        }
        {
            const size_t n = static_cast<size_t>(mod_in_.functions().size());
            mod_out_.functions.Reserve(n);
            for (auto& fn_in : mod_in_.functions()) {
                mod_out_.functions.Push(CreateFunction(fn_in));
            }
        }
        {
            const size_t n = static_cast<size_t>(mod_in_.blocks().size());
            blocks_.Reserve(n);
            for (size_t i = 0; i < n; i++) {
                auto id = static_cast<uint32_t>(i + 1);
                if (id == mod_in_.root_block()) {
                    blocks_.Push(mod_out_.root_block);
                } else {
                    auto& block_in = mod_in_.blocks()[static_cast<int>(i)];
                    blocks_.Push(CreateBlock(block_in));
                }
            }
        }
        {
            const size_t n = static_cast<size_t>(mod_in_.constant_values().size());
            constant_values_.Reserve(n);
            for (auto& value_in : mod_in_.constant_values()) {
                constant_values_.Push(CreateConstantValue(value_in));
            }
        }
        {
            const size_t n = static_cast<size_t>(mod_in_.values().size());
            values_.Reserve(n);
            for (auto& value_in : mod_in_.values()) {
                values_.Push(CreateValue(value_in));
            }
        }
        for (size_t i = 0, n = static_cast<size_t>(mod_in_.functions().size()); i < n; i++) {
            PopulateFunction(mod_out_.functions[i], mod_in_.functions()[static_cast<int>(i)]);
        }
        for (size_t i = 0, n = static_cast<size_t>(mod_in_.blocks().size()); i < n; i++) {
            PopulateBlock(blocks_[i], mod_in_.blocks()[static_cast<int>(i)]);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////
    ir::Function* CreateFunction(const pb::Function&) {
        return mod_out_.values.Create<ir::Function>();
    }

    void PopulateFunction(ir::Function* fn_out, const pb::Function& fn_in) {
        if (!fn_in.name().empty()) {
            mod_out_.SetName(fn_out, fn_in.name());
        }
        fn_out->SetReturnType(Type(fn_in.return_type()));
        if (fn_in.has_pipeline_stage()) {
            fn_out->SetStage(PipelineStage(fn_in.pipeline_stage()));
        }
        if (fn_in.has_workgroup_size()) {
            auto& wg_size_in = fn_in.workgroup_size();
            fn_out->SetWorkgroupSize(wg_size_in.x(), wg_size_in.y(), wg_size_in.z());
        }

        Vector<FunctionParam*, 8> params_out;
        for (auto param_in : fn_in.parameters()) {
            params_out.Push(ValueAs<ir::FunctionParam>(param_in));
        }
        fn_out->SetParams(std::move(params_out));
        fn_out->SetBlock(Block(fn_in.block()));
    }

    ir::Function* Function(uint32_t id) { return id > 0 ? mod_out_.functions[id - 1] : nullptr; }

    Function::PipelineStage PipelineStage(pb::PipelineStage stage) {
        switch (stage) {
            case pb::PipelineStage::Compute:
                return Function::PipelineStage::kCompute;
            case pb::PipelineStage::Fragment:
                return Function::PipelineStage::kFragment;
            case pb::PipelineStage::Vertex:
                return Function::PipelineStage::kVertex;
            default:
                TINT_ICE() << "unhandled PipelineStage: " << stage;
                return Function::PipelineStage::kCompute;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Blocks
    ////////////////////////////////////////////////////////////////////////////
    ir::Block* CreateBlock(const pb::Block&) { return b.Block(); }

    ir::Block* PopulateBlock(ir::Block* block_out, const pb::Block& block_in) {
        for (auto& inst : block_in.instructions()) {
            block_out->Append(Instruction(inst));
        }
        return block_out;
    }

    ir::Block* Block(uint32_t id) { return id > 0 ? blocks_[id - 1] : nullptr; }

    ////////////////////////////////////////////////////////////////////////////
    // Instructions
    ////////////////////////////////////////////////////////////////////////////
    ir::Instruction* Instruction(const pb::Instruction& inst_in) {
        ir::Instruction* inst_out = nullptr;
        switch (inst_in.kind_case()) {
            case pb::Instruction::KindCase::kAccess:
                inst_out = CreateInstructionAccess(inst_in.access());
                break;
            case pb::Instruction::KindCase::kBinary:
                inst_out = CreateInstructionBinary(inst_in.binary());
                break;
            case pb::Instruction::KindCase::kConstruct:
                inst_out = CreateInstructionConstruct(inst_in.construct());
                break;
            case pb::Instruction::KindCase::kDiscard:
                inst_out = CreateInstructionDiscard(inst_in.discard());
                break;
            case pb::Instruction::KindCase::kLet:
                inst_out = CreateInstructionLet(inst_in.let());
                break;
            case pb::Instruction::KindCase::kLoad:
                inst_out = CreateInstructionLoad(inst_in.load());
                break;
            case pb::Instruction::KindCase::kLoadVectorElement:
                inst_out = CreateInstructionLoadVectorElement(inst_in.load_vector_element());
                break;
            case pb::Instruction::KindCase::kReturn:
                inst_out = CreateInstructionReturn(inst_in.return_());
                break;
            case pb::Instruction::KindCase::kStore:
                inst_out = CreateInstructionStore(inst_in.store());
                break;
            case pb::Instruction::KindCase::kStoreVectorElement:
                inst_out = CreateInstructionStoreVectorElement(inst_in.store_vector_element());
                break;
            case pb::Instruction::KindCase::kSwizzle:
                inst_out = CreateInstructionSwizzle(inst_in.swizzle());
                break;
            case pb::Instruction::KindCase::kUnary:
                inst_out = CreateInstructionUnary(inst_in.unary());
                break;
            case pb::Instruction::KindCase::kUserCall:
                inst_out = CreateInstructionUserCall(inst_in.user_call());
                break;
            case pb::Instruction::KindCase::kVar:
                inst_out = CreateInstructionVar(inst_in.var());
                break;
            default:
                TINT_UNIMPLEMENTED() << inst_in.kind_case();
                break;
        }
        TINT_ASSERT_OR_RETURN_VALUE(inst_out, nullptr);

        Vector<ir::Value*, 4> operands;
        for (auto id : inst_in.operands()) {
            operands.Push(Value(id));
        }
        inst_out->SetOperands(std::move(operands));

        Vector<ir::InstructionResult*, 4> results;
        for (auto id : inst_in.results()) {
            results.Push(ValueAs<ir::InstructionResult>(id));
        }
        inst_out->SetResults(std::move(results));

        return inst_out;
    }

    ir::Access* CreateInstructionAccess(const pb::InstructionAccess&) {
        return mod_out_.instructions.Create<ir::Access>();
    }

    ir::Binary* CreateInstructionBinary(const pb::InstructionBinary& binary_in) {
        auto* binary_out = mod_out_.instructions.Create<ir::Binary>();
        binary_out->SetOp(BinaryOp(binary_in.op()));
        return binary_out;
    }

    ir::Construct* CreateInstructionConstruct(const pb::InstructionConstruct&) {
        return mod_out_.instructions.Create<ir::Construct>();
    }

    ir::Discard* CreateInstructionDiscard(const pb::InstructionDiscard&) {
        return mod_out_.instructions.Create<ir::Discard>();
    }

    ir::Let* CreateInstructionLet(const pb::InstructionLet&) {
        return mod_out_.instructions.Create<ir::Let>();
    }

    ir::Load* CreateInstructionLoad(const pb::InstructionLoad&) {
        return mod_out_.instructions.Create<ir::Load>();
    }

    ir::LoadVectorElement* CreateInstructionLoadVectorElement(
        const pb::InstructionLoadVectorElement&) {
        return mod_out_.instructions.Create<ir::LoadVectorElement>();
    }

    ir::Return* CreateInstructionReturn(const pb::InstructionReturn&) {
        return mod_out_.instructions.Create<ir::Return>();
    }

    ir::Store* CreateInstructionStore(const pb::InstructionStore&) {
        return mod_out_.instructions.Create<ir::Store>();
    }

    ir::StoreVectorElement* CreateInstructionStoreVectorElement(
        const pb::InstructionStoreVectorElement&) {
        return mod_out_.instructions.Create<ir::StoreVectorElement>();
    }

    ir::Swizzle* CreateInstructionSwizzle(const pb::InstructionSwizzle& swizzle_in) {
        auto* swizzle_out = mod_out_.instructions.Create<ir::Swizzle>();
        Vector<uint32_t, 4> indices;
        for (auto idx : swizzle_in.indices()) {
            indices.Push(idx);
        }
        swizzle_out->SetIndices(indices);
        return swizzle_out;
    }

    ir::Unary* CreateInstructionUnary(const pb::InstructionUnary& unary_in) {
        auto* unary_out = mod_out_.instructions.Create<ir::Unary>();
        unary_out->SetOp(UnaryOp(unary_in.op()));
        return unary_out;
    }

    ir::UserCall* CreateInstructionUserCall(const pb::InstructionUserCall&) {
        return mod_out_.instructions.Create<ir::UserCall>();
    }

    ir::Var* CreateInstructionVar(const pb::InstructionVar& var_in) {
        auto* var_out = mod_out_.instructions.Create<ir::Var>();
        if (var_in.has_binding_point()) {
            auto& bp_in = var_in.binding_point();
            var_out->SetBindingPoint(bp_in.group(), bp_in.binding());
        }
        return var_out;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////
    const type::Type* CreateType(const pb::Type type_in) {
        switch (type_in.kind_case()) {
            case pb::Type::KindCase::kBasic:
                return CreateTypeBasic(type_in.basic());
            case pb::Type::KindCase::kVector:
                return CreateTypeVector(type_in.vector());
            case pb::Type::KindCase::kMatrix:
                return CreateTypeMatrix(type_in.matrix());
            case pb::Type::KindCase::kPointer:
                return CreateTypePointer(type_in.pointer());
            case pb::Type::KindCase::kArray:
                return CreateTypeArray(type_in.array());
            case pb::Type::KindCase::kAtomic:
                TINT_UNIMPLEMENTED() << type_in.kind_case();
                return nullptr;

            case pb::Type::KindCase::KIND_NOT_SET:
                break;
        }
        TINT_ICE() << "invalid TypeDecl.kind";
        return nullptr;
    }

    const type::Type* CreateTypeBasic(pb::TypeBasic basic_in) {
        switch (basic_in) {
            case pb::TypeBasic::void_:
                return mod_out_.Types().Get<void>();
            case pb::TypeBasic::bool_:
                return mod_out_.Types().Get<bool>();
            case pb::TypeBasic::i32:
                return mod_out_.Types().Get<i32>();
            case pb::TypeBasic::u32:
                return mod_out_.Types().Get<u32>();
            case pb::TypeBasic::f32:
                return mod_out_.Types().Get<f32>();
            case pb::TypeBasic::f16:
                return mod_out_.Types().Get<f16>();
            default:
                TINT_ICE() << "invalid TypeBasic: " << basic_in;
                return nullptr;
        }
    }

    const type::Vector* CreateTypeVector(const pb::TypeVector& vector_in) {
        auto* el_ty = Type(vector_in.element_type());
        return mod_out_.Types().vec(el_ty, vector_in.width());
    }

    const type::Matrix* CreateTypeMatrix(const pb::TypeMatrix& matrix_in) {
        auto* el_ty = Type(matrix_in.element_type());
        auto* column_ty = mod_out_.Types().vec(el_ty, matrix_in.num_rows());
        return mod_out_.Types().mat(column_ty, matrix_in.num_columns());
    }

    const type::Pointer* CreateTypePointer(const pb::TypePointer& pointer_in) {
        auto address_space = AddressSpace(pointer_in.address_space());
        auto* store_ty = Type(pointer_in.store_type());
        auto access = Access(pointer_in.access());
        return mod_out_.Types().ptr(address_space, store_ty, access);
    }

    const type::Array* CreateTypeArray(const pb::TypeArray& array_in) {
        auto* element = Type(array_in.element());
        uint32_t stride = static_cast<uint32_t>(array_in.stride());
        uint32_t count = static_cast<uint32_t>(array_in.count());
        return mod_out_.Types().array(element, count, stride);
    }

    const type::Type* Type(size_t id) { return id > 0 ? types_[id - 1] : nullptr; }

    ////////////////////////////////////////////////////////////////////////////
    // Values
    ////////////////////////////////////////////////////////////////////////////
    ir::Value* CreateValue(const pb::Value& value_in) {
        ir::Value* value_out = nullptr;
        switch (value_in.kind_case()) {
            case pb::Value::KindCase::kFunction: {
                value_out = Function(value_in.function());
                break;
            }
            case pb::Value::KindCase::kInstructionResult: {
                auto& res_in = value_in.instruction_result();
                auto* type = Type(res_in.type());
                value_out = b.InstructionResult(type);
                if (res_in.has_name()) {
                    mod_out_.SetName(value_out, res_in.name());
                }
                break;
            }
            case pb::Value::KindCase::kFunctionParameter: {
                auto& param_in = value_in.function_parameter();
                auto* type = Type(param_in.type());
                value_out = b.FunctionParam(type);
                if (param_in.has_name()) {
                    mod_out_.SetName(value_out, param_in.name());
                }
                break;
            }
            case pb::Value::KindCase::kConstant: {
                value_out = b.Constant(ConstantValue(value_in.constant()));
                break;
            }
            default:
                TINT_ICE() << "invalid TypeDecl.kind: " << value_in.kind_case();
                return nullptr;
        }
        return value_out;
    }

    ir::Value* Value(uint32_t id) { return id > 0 ? values_[id - 1] : nullptr; }

    template <typename T>
    T* ValueAs(uint32_t id) {
        auto* value = Value(id);
        if (auto cast = value->As<T>(); TINT_LIKELY(cast)) {
            return cast;
        }
        TINT_ICE() << "Value " << id << " is " << (value ? value->TypeInfo().name : "<null>")
                   << " expected " << TypeInfo::Of<T>().name;
        return nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////
    // ConstantValues
    ////////////////////////////////////////////////////////////////////////////
    const core::constant::Value* CreateConstantValue(const pb::ConstantValue& value_in) {
        switch (value_in.kind_case()) {
            case pb::ConstantValue::KindCase::kScalar:
                return CreateConstantScalar(value_in.scalar());
            case pb::ConstantValue::KindCase::kComposite:
                return CreateConstantComposite(value_in.composite());
            case pb::ConstantValue::KindCase::kSplat:
                return CreateConstantSplat(value_in.splat());
            default:
                TINT_ICE() << "invalid ConstantValue.kind: " << value_in.kind_case();
                return nullptr;
        }
    }

    const core::constant::Value* CreateConstantScalar(const pb::ConstantValueScalar& value_in) {
        switch (value_in.kind_case()) {
            case pb::ConstantValueScalar::KindCase::kBool:
                return b.ConstantValue(value_in.bool_());
            case pb::ConstantValueScalar::KindCase::kI32:
                return b.ConstantValue(i32(value_in.i32()));
            case pb::ConstantValueScalar::KindCase::kU32:
                return b.ConstantValue(u32(value_in.u32()));
            case pb::ConstantValueScalar::KindCase::kF32:
                return b.ConstantValue(f32(value_in.f32()));
            case pb::ConstantValueScalar::KindCase::kF16:
                return b.ConstantValue(f16(value_in.f16()));
            default:
                TINT_ICE() << "invalid ConstantValueScalar.kind: " << value_in.kind_case();
                return nullptr;
        }
    }

    const core::constant::Value* CreateConstantComposite(
        const pb::ConstantValueComposite& composite_in) {
        auto* type = Type(composite_in.type());
        Vector<const core::constant::Value*, 8> elements_out;
        for (auto element_id : composite_in.elements()) {
            elements_out.Push(ConstantValue(element_id));
        }
        return mod_out_.constant_values.Composite(type, std::move(elements_out));
    }

    const core::constant::Value* CreateConstantSplat(const pb::ConstantValueSplat& splat_in) {
        auto* type = Type(splat_in.type());
        auto* elem = ConstantValue(splat_in.elements());
        return mod_out_.constant_values.Splat(type, elem, splat_in.count());
    }

    const core::constant::Value* ConstantValue(uint32_t id) {
        return id > 0 ? constant_values_[id - 1] : nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Enums
    ////////////////////////////////////////////////////////////////////////////
    core::AddressSpace AddressSpace(pb::AddressSpace in) {
        switch (in) {
            case pb::AddressSpace::function:
                return core::AddressSpace::kFunction;
            case pb::AddressSpace::handle:
                return core::AddressSpace::kHandle;
            case pb::AddressSpace::pixel_local:
                return core::AddressSpace::kPixelLocal;
            case pb::AddressSpace::private_:
                return core::AddressSpace::kPrivate;
            case pb::AddressSpace::push_constant:
                return core::AddressSpace::kPushConstant;
            case pb::AddressSpace::storage:
                return core::AddressSpace::kStorage;
            case pb::AddressSpace::uniform:
                return core::AddressSpace::kUniform;
            case pb::AddressSpace::workgroup:
                return core::AddressSpace::kWorkgroup;
            default:
                TINT_ICE() << "invalid AddressSpace: " << in;
                return core::AddressSpace::kUndefined;
        }
    }

    core::Access Access(pb::AccessControl in) {
        switch (in) {
            case pb::AccessControl::read:
                return core::Access::kRead;
            case pb::AccessControl::write:
                return core::Access::kWrite;
            case pb::AccessControl::read_write:
                return core::Access::kReadWrite;
            default:
                TINT_ICE() << "invalid Access: " << in;
                return core::Access::kUndefined;
        }
    }

    core::ir::UnaryOp UnaryOp(pb::UnaryOp in) {
        switch (in) {
            case pb::UnaryOp::complement:
                return core::ir::UnaryOp::kComplement;
            case pb::UnaryOp::negation:
                return core::ir::UnaryOp::kNegation;

            default:
                TINT_ICE() << "invalid UnaryOp: " << in;
                return core::ir::UnaryOp::kComplement;
        }
    }

    core::ir::BinaryOp BinaryOp(pb::BinaryOp in) {
        switch (in) {
            case pb::BinaryOp::add_:
                return core::ir::BinaryOp::kAdd;
            case pb::BinaryOp::subtract:
                return core::ir::BinaryOp::kSubtract;
            case pb::BinaryOp::multiply:
                return core::ir::BinaryOp::kMultiply;
            case pb::BinaryOp::divide:
                return core::ir::BinaryOp::kDivide;
            case pb::BinaryOp::modulo:
                return core::ir::BinaryOp::kModulo;
            case pb::BinaryOp::and_:
                return core::ir::BinaryOp::kAnd;
            case pb::BinaryOp::or_:
                return core::ir::BinaryOp::kOr;
            case pb::BinaryOp::xor_:
                return core::ir::BinaryOp::kXor;
            case pb::BinaryOp::equal:
                return core::ir::BinaryOp::kEqual;
            case pb::BinaryOp::not_equal:
                return core::ir::BinaryOp::kNotEqual;
            case pb::BinaryOp::less_than:
                return core::ir::BinaryOp::kLessThan;
            case pb::BinaryOp::greater_than:
                return core::ir::BinaryOp::kGreaterThan;
            case pb::BinaryOp::less_than_equal:
                return core::ir::BinaryOp::kLessThanEqual;
            case pb::BinaryOp::greater_than_equal:
                return core::ir::BinaryOp::kGreaterThanEqual;
            case pb::BinaryOp::shift_left:
                return core::ir::BinaryOp::kShiftLeft;
            case pb::BinaryOp::shift_right:
                return core::ir::BinaryOp::kShiftRight;

            default:
                TINT_ICE() << "invalid BinaryOp: " << in;
                return core::ir::BinaryOp::kAdd;
        }
    }
};

}  // namespace

Result<Module> Decode(Slice<const std::byte> encoded) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    pb::Module mod_in;
    if (!mod_in.ParseFromArray(encoded.data, static_cast<int>(encoded.len))) {
        return Failure{"failed to deserialize protobuf"};
    }

    Module mod_out;
    Decoder{mod_in, mod_out}.Decode();

    return mod_out;
}

}  // namespace tint::core::ir::binary
