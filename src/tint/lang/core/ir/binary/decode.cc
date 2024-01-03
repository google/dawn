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
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
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

    Vector<ir::ExitIf*, 32> exit_ifs_{};
    Vector<ir::ExitSwitch*, 32> exit_switches_{};
    Vector<ir::ExitLoop*, 32> exit_loops_{};
    Vector<ir::NextIteration*, 32> next_iterations_{};
    Vector<ir::BreakIf*, 32> break_ifs_{};
    Vector<ir::Continue*, 32> continues_{};

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

        for (auto* exit : exit_ifs_) {
            InferControlInstruction(exit, &ExitIf::SetIf);
        }
        for (auto* exit : exit_switches_) {
            InferControlInstruction(exit, &ExitSwitch::SetSwitch);
        }
        for (auto* exit : exit_loops_) {
            InferControlInstruction(exit, &ExitLoop::SetLoop);
        }
        for (auto* break_ifs : break_ifs_) {
            InferControlInstruction(break_ifs, &BreakIf::SetLoop);
        }
        for (auto* next_iters : next_iterations_) {
            InferControlInstruction(next_iters, &NextIteration::SetLoop);
        }
        for (auto* cont : continues_) {
            InferControlInstruction(cont, &Continue::SetLoop);
        }
    }

    template <typename EXIT, typename CTRL_INST>
    void InferControlInstruction(EXIT* exit, void (EXIT::*set)(CTRL_INST*)) {
        for (auto* block = exit->Block(); block;) {
            auto* parent = block->Parent();
            if (!parent) {
                break;
            }
            if (auto* ctrl_inst = parent->template As<CTRL_INST>()) {
                (exit->*set)(ctrl_inst);
                break;
            }
            block = parent->Block();
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
        if (fn_in.has_return_location()) {
            fn_out->SetReturnLocation(Location(fn_in.return_location()));
        }
        if (fn_in.has_return_builtin()) {
            fn_out->SetReturnBuiltin(BuiltinValue(fn_in.return_builtin()));
        }
        if (fn_in.return_invariant()) {
            fn_out->SetReturnInvariant(true);
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
    ir::Block* CreateBlock(const pb::Block& block_in) {
        return block_in.is_multi_in() ? b.MultiInBlock() : b.Block();
    }

    void PopulateBlock(ir::Block* block_out, const pb::Block& block_in) {
        if (block_in.is_multi_in()) {
            Vector<ir::BlockParam*, 8> params;
            for (auto param : block_in.parameters()) {
                params.Push(ValueAs<BlockParam>(param));
            }
            block_out->As<ir::MultiInBlock>()->SetParams(std::move(params));
        }
        for (auto& inst : block_in.instructions()) {
            block_out->Append(Instruction(inst));
        }
    }

    ir::Block* Block(uint32_t id) { return id > 0 ? blocks_[id - 1] : nullptr; }

    template <typename T>
    T* BlockAs(uint32_t id) {
        auto* block = Block(id);
        if (auto cast = block->As<T>(); TINT_LIKELY(cast)) {
            return cast;
        }
        TINT_ICE() << "block " << id << " is " << (block ? block->TypeInfo().name : "<null>")
                   << " expected " << TypeInfo::Of<T>().name;
        return nullptr;
    }

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
            case pb::Instruction::KindCase::kBitcast:
                inst_out = CreateInstructionBitcast(inst_in.bitcast());
                break;
            case pb::Instruction::KindCase::kBreakIf:
                inst_out = CreateInstructionBreakIf(inst_in.break_if());
                break;
            case pb::Instruction::KindCase::kBuiltinCall:
                inst_out = CreateInstructionBuiltinCall(inst_in.builtin_call());
                break;
            case pb::Instruction::KindCase::kConstruct:
                inst_out = CreateInstructionConstruct(inst_in.construct());
                break;
            case pb::Instruction::KindCase::kContinue:
                inst_out = CreateInstructionContinue(inst_in.continue_());
                break;
            case pb::Instruction::KindCase::kConvert:
                inst_out = CreateInstructionConvert(inst_in.convert());
                break;
            case pb::Instruction::KindCase::kExitIf:
                inst_out = CreateInstructionExitIf(inst_in.exit_if());
                break;
            case pb::Instruction::KindCase::kExitLoop:
                inst_out = CreateInstructionExitLoop(inst_in.exit_loop());
                break;
            case pb::Instruction::KindCase::kExitSwitch:
                inst_out = CreateInstructionExitSwitch(inst_in.exit_switch());
                break;
            case pb::Instruction::KindCase::kDiscard:
                inst_out = CreateInstructionDiscard(inst_in.discard());
                break;
            case pb::Instruction::KindCase::kIf:
                inst_out = CreateInstructionIf(inst_in.if_());
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
            case pb::Instruction::KindCase::kLoop:
                inst_out = CreateInstructionLoop(inst_in.loop());
                break;
            case pb::Instruction::KindCase::kNextIteration:
                inst_out = CreateInstructionNextIteration(inst_in.next_iteration());
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
            case pb::Instruction::KindCase::kSwitch:
                inst_out = CreateInstructionSwitch(inst_in.switch_());
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
            case pb::Instruction::KindCase::kUnreachable:
                inst_out = CreateInstructionUnreachable(inst_in.unreachable());
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

    ir::Bitcast* CreateInstructionBitcast(const pb::InstructionBitcast&) {
        return mod_out_.instructions.Create<ir::Bitcast>();
    }

    ir::BreakIf* CreateInstructionBreakIf(const pb::InstructionBreakIf&) {
        auto* break_if_out = mod_out_.instructions.Create<ir::BreakIf>();
        break_ifs_.Push(break_if_out);
        return break_if_out;
    }

    ir::CoreBuiltinCall* CreateInstructionBuiltinCall(const pb::InstructionBuiltinCall& call_in) {
        auto* call_out = mod_out_.instructions.Create<ir::CoreBuiltinCall>();
        call_out->SetFunc(BuiltinFn(call_in.builtin()));
        return call_out;
    }

    ir::Construct* CreateInstructionConstruct(const pb::InstructionConstruct&) {
        return mod_out_.instructions.Create<ir::Construct>();
    }

    ir::Continue* CreateInstructionContinue(const pb::InstructionContinue&) {
        auto* continue_ = mod_out_.instructions.Create<ir::Continue>();
        continues_.Push(continue_);
        return continue_;
    }

    ir::Convert* CreateInstructionConvert(const pb::InstructionConvert&) {
        return mod_out_.instructions.Create<ir::Convert>();
    }

    ir::ExitIf* CreateInstructionExitIf(const pb::InstructionExitIf&) {
        auto* exit_out = mod_out_.instructions.Create<ir::ExitIf>();
        exit_ifs_.Push(exit_out);
        return exit_out;
    }

    ir::ExitLoop* CreateInstructionExitLoop(const pb::InstructionExitLoop&) {
        auto* exit_out = mod_out_.instructions.Create<ir::ExitLoop>();
        exit_loops_.Push(exit_out);
        return exit_out;
    }

    ir::ExitSwitch* CreateInstructionExitSwitch(const pb::InstructionExitSwitch&) {
        auto* exit_out = mod_out_.instructions.Create<ir::ExitSwitch>();
        exit_switches_.Push(exit_out);
        return exit_out;
    }

    ir::Discard* CreateInstructionDiscard(const pb::InstructionDiscard&) {
        return mod_out_.instructions.Create<ir::Discard>();
    }

    ir::If* CreateInstructionIf(const pb::InstructionIf& if_in) {
        auto* if_out = mod_out_.instructions.Create<ir::If>();
        if (if_in.has_true_()) {
            if_out->SetTrue(Block(if_in.true_()));
        }
        if (if_in.has_false_()) {
            if_out->SetFalse(Block(if_in.false_()));
        }
        return if_out;
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

    ir::Loop* CreateInstructionLoop(const pb::InstructionLoop& loop_in) {
        auto* loop_out = mod_out_.instructions.Create<ir::Loop>();
        if (loop_in.has_initalizer()) {
            loop_out->SetInitializer(Block(loop_in.initalizer()));
        } else {
            loop_out->SetInitializer(mod_out_.blocks.Create());
        }
        loop_out->SetBody(BlockAs<ir::MultiInBlock>(loop_in.body()));
        if (loop_in.has_continuing()) {
            loop_out->SetContinuing(BlockAs<ir::MultiInBlock>(loop_in.continuing()));
        } else {
            loop_out->SetContinuing(mod_out_.blocks.Create<ir::MultiInBlock>());
        }
        return loop_out;
    }

    ir::NextIteration* CreateInstructionNextIteration(const pb::InstructionNextIteration&) {
        auto* next_it_out = mod_out_.instructions.Create<ir::NextIteration>();
        next_iterations_.Push(next_it_out);
        return next_it_out;
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

    ir::Switch* CreateInstructionSwitch(const pb::InstructionSwitch& switch_in) {
        auto* switch_out = mod_out_.instructions.Create<ir::Switch>();
        for (auto& case_in : switch_in.cases()) {
            ir::Switch::Case case_out{};
            case_out.block = Block(case_in.block());
            case_out.block->SetParent(switch_out);
            for (auto selector_in : case_in.selectors()) {
                ir::Switch::CaseSelector selector_out{};
                selector_out.val = b.Constant(ConstantValue(selector_in));
                case_out.selectors.Push(std::move(selector_out));
            }
            if (case_in.is_default()) {
                ir::Switch::CaseSelector selector_out{};
                case_out.selectors.Push(std::move(selector_out));
            }
            switch_out->Cases().Push(std::move(case_out));
        }
        return switch_out;
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

    ir::Unreachable* CreateInstructionUnreachable(const pb::InstructionUnreachable&) {
        return b.Unreachable();
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
            case pb::Type::KindCase::kStruct:
                return CreateTypeStruct(type_in.struct_());
            case pb::Type::KindCase::kAtomic:
                return CreateTypeAtomic(type_in.atomic());
            case pb::Type::KindCase::kArray:
                return CreateTypeArray(type_in.array());
            case pb::Type::KindCase::kDepthTexture:
                return CreateTypeDepthTexture(type_in.depth_texture());
            case pb::Type::KindCase::kSampledTexture:
                return CreateTypeSampledTexture(type_in.sampled_texture());
            case pb::Type::KindCase::kMultisampledTexture:
                return CreateTypeMultisampledTexture(type_in.multisampled_texture());
            case pb::Type::KindCase::kDepthMultisampledTexture:
                return CreateTypeDepthMultisampledTexture(type_in.depth_multisampled_texture());
            case pb::Type::KindCase::kStorageTexture:
                return CreateTypeStorageTexture(type_in.storage_texture());
            case pb::Type::KindCase::kExternalTexture:
                return CreateTypeExternalTexture(type_in.external_texture());
            case pb::Type::KindCase::kSampler:
                return CreateTypeSampler(type_in.sampler());
            default:
                break;
        }
        TINT_ICE() << type_in.kind_case();
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
        auto access = AccessControl(pointer_in.access());
        return mod_out_.Types().ptr(address_space, store_ty, access);
    }

    const type::Struct* CreateTypeStruct(const pb::TypeStruct& struct_in) {
        Vector<const core::type::StructMember*, 8> members_out;
        uint32_t offset = 0;
        for (auto& member_in : struct_in.member()) {
            auto symbol = mod_out_.symbols.Register(member_in.name());
            auto* type = Type(member_in.type());
            auto index = static_cast<uint32_t>(members_out.Length());
            auto align = member_in.align();
            auto size = member_in.size();
            core::type::StructMemberAttributes attributes_out{};
            if (member_in.has_attributes()) {
                auto& attributes_in = member_in.attributes();
                if (attributes_in.has_location()) {
                    attributes_out.location = attributes_in.location();
                }
                if (attributes_in.has_index()) {
                    attributes_out.index = attributes_in.index();
                }
                if (attributes_in.has_color()) {
                    attributes_out.color = attributes_in.color();
                }
                if (attributes_in.has_builtin()) {
                    attributes_out.builtin = BuiltinValue(attributes_in.builtin());
                }
                if (attributes_in.has_interpolation()) {
                    auto& interpolation_in = attributes_in.interpolation();
                    attributes_out.interpolation = Interpolation(interpolation_in);
                }
                attributes_out.invariant = attributes_in.invariant();
            }
            offset = RoundUp(align, offset);
            auto* member_out = mod_out_.Types().Get<core::type::StructMember>(
                symbol, type, index, offset, align, size, std::move(attributes_out));
            offset += size;
            members_out.Push(member_out);
        }
        auto name = mod_out_.symbols.Register(struct_in.name());
        return mod_out_.Types().Struct(name, std::move(members_out));
    }

    const type::Atomic* CreateTypeAtomic(const pb::TypeAtomic& atomic_in) {
        return mod_out_.Types().atomic(Type(atomic_in.type()));
    }

    const type::Array* CreateTypeArray(const pb::TypeArray& array_in) {
        auto* element = Type(array_in.element());
        uint32_t stride = static_cast<uint32_t>(array_in.stride());
        uint32_t count = static_cast<uint32_t>(array_in.count());
        return count > 0 ? mod_out_.Types().array(element, count, stride)
                         : mod_out_.Types().runtime_array(element, stride);
    }

    const type::DepthTexture* CreateTypeDepthTexture(const pb::TypeDepthTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        return mod_out_.Types().Get<type::DepthTexture>(dimension);
    }

    const type::SampledTexture* CreateTypeSampledTexture(const pb::TypeSampledTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        auto sub_type = Type(texture_in.sub_type());
        return mod_out_.Types().Get<type::SampledTexture>(dimension, sub_type);
    }

    const type::MultisampledTexture* CreateTypeMultisampledTexture(
        const pb::TypeMultisampledTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        auto sub_type = Type(texture_in.sub_type());
        return mod_out_.Types().Get<type::MultisampledTexture>(dimension, sub_type);
    }

    const type::DepthMultisampledTexture* CreateTypeDepthMultisampledTexture(
        const pb::TypeDepthMultisampledTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        return mod_out_.Types().Get<type::DepthMultisampledTexture>(dimension);
    }

    const type::StorageTexture* CreateTypeStorageTexture(const pb::TypeStorageTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        auto texel_format = TexelFormat(texture_in.texel_format());
        auto access = AccessControl(texture_in.access());
        return mod_out_.Types().Get<type::StorageTexture>(
            dimension, texel_format, access,
            type::StorageTexture::SubtypeFor(texel_format, b.ir.Types()));
    }

    const type::ExternalTexture* CreateTypeExternalTexture(const pb::TypeExternalTexture&) {
        return mod_out_.Types().Get<type::ExternalTexture>();
    }

    const type::Sampler* CreateTypeSampler(const pb::TypeSampler& sampler_in) {
        auto kind = SamplerKind(sampler_in.kind());
        return mod_out_.Types().Get<type::Sampler>(kind);
    }

    const type::Type* Type(size_t id) { return id > 0 ? types_[id - 1] : nullptr; }

    ////////////////////////////////////////////////////////////////////////////
    // Values
    ////////////////////////////////////////////////////////////////////////////
    ir::Value* CreateValue(const pb::Value& value_in) {
        ir::Value* value_out = nullptr;
        switch (value_in.kind_case()) {
            case pb::Value::KindCase::kFunction:
                value_out = Function(value_in.function());
                break;
            case pb::Value::KindCase::kInstructionResult:
                value_out = InstructionResult(value_in.instruction_result());
                break;
            case pb::Value::KindCase::kFunctionParameter:
                value_out = FunctionParameter(value_in.function_parameter());
                break;
            case pb::Value::KindCase::kBlockParameter:
                value_out = BlockParameter(value_in.block_parameter());
                break;
            case pb::Value::KindCase::kConstant:
                value_out = b.Constant(ConstantValue(value_in.constant()));
                break;
            default:
                TINT_ICE() << "invalid TypeDecl.kind: " << value_in.kind_case();
                return nullptr;
        }
        return value_out;
    }

    ir::InstructionResult* InstructionResult(const pb::InstructionResult& res_in) {
        auto* type = Type(res_in.type());
        auto* res_out = b.InstructionResult(type);
        if (res_in.has_name()) {
            mod_out_.SetName(res_out, res_in.name());
        }
        return res_out;
    }

    ir::FunctionParam* FunctionParameter(const pb::FunctionParameter& param_in) {
        auto* type = Type(param_in.type());
        auto* param_out = b.FunctionParam(type);
        if (param_in.has_name()) {
            mod_out_.SetName(param_out, param_in.name());
        }

        if (param_in.has_attributes()) {
            auto& attrs_in = param_in.attributes();
            if (attrs_in.has_binding_point()) {
                auto& bp_in = attrs_in.binding_point();
                param_out->SetBindingPoint(bp_in.group(), bp_in.binding());
            }
            if (attrs_in.has_location()) {
                param_out->SetLocation(Location(attrs_in.location()));
            }
            if (attrs_in.has_builtin()) {
                param_out->SetBuiltin(BuiltinValue(attrs_in.builtin()));
            }
            if (attrs_in.invariant()) {
                param_out->SetInvariant(true);
            }
        }

        return param_out;
    }

    ir::BlockParam* BlockParameter(const pb::BlockParameter& param_in) {
        auto* type = Type(param_in.type());
        auto* param_out = b.BlockParam(type);
        if (param_in.has_name()) {
            mod_out_.SetName(param_out, param_in.name());
        }
        return param_out;
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
    // Attributes
    ////////////////////////////////////////////////////////////////////////////
    ir::Location Location(const pb::Location& location_in) {
        core::ir::Location location_out{};
        location_out.value = location_in.value();
        if (location_in.has_interpolation()) {
            location_out.interpolation = Interpolation(location_in.interpolation());
        }
        return location_out;
    }

    core::Interpolation Interpolation(const pb::Interpolation& interpolation_in) {
        core::Interpolation interpolation_out{};
        interpolation_out.type = InterpolationType(interpolation_in.type());
        if (interpolation_in.has_sampling()) {
            interpolation_out.sampling = InterpolationSampling(interpolation_in.sampling());
        }
        return interpolation_out;
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

    core::Access AccessControl(pb::AccessControl in) {
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

    core::type::TextureDimension TextureDimension(pb::TextureDimension in) {
        switch (in) {
            case pb::TextureDimension::_1d:
                return core::type::TextureDimension::k1d;
            case pb::TextureDimension::_2d:
                return core::type::TextureDimension::k2d;
            case pb::TextureDimension::_2d_array:
                return core::type::TextureDimension::k2dArray;
            case pb::TextureDimension::_3d:
                return core::type::TextureDimension::k3d;
            case pb::TextureDimension::cube:
                return core::type::TextureDimension::kCube;
            case pb::TextureDimension::cube_array:
                return core::type::TextureDimension::kCubeArray;
            default:
                break;
        }

        TINT_ICE() << "invalid TextureDimension: " << in;
        return core::type::TextureDimension::k1d;
    }

    core::TexelFormat TexelFormat(pb::TexelFormat in) {
        switch (in) {
            case pb::TexelFormat::bgra8_unorm:
                return core::TexelFormat::kBgra8Unorm;
            case pb::TexelFormat::r32_float:
                return core::TexelFormat::kR32Float;
            case pb::TexelFormat::r32_sint:
                return core::TexelFormat::kR32Sint;
            case pb::TexelFormat::r32_uint:
                return core::TexelFormat::kR32Uint;
            case pb::TexelFormat::rg32_float:
                return core::TexelFormat::kRg32Float;
            case pb::TexelFormat::rg32_sint:
                return core::TexelFormat::kRg32Sint;
            case pb::TexelFormat::rg32_uint:
                return core::TexelFormat::kRg32Uint;
            case pb::TexelFormat::rgba16_float:
                return core::TexelFormat::kRgba16Float;
            case pb::TexelFormat::rgba16_sint:
                return core::TexelFormat::kRgba16Sint;
            case pb::TexelFormat::rgba16_uint:
                return core::TexelFormat::kRgba16Uint;
            case pb::TexelFormat::rgba32_float:
                return core::TexelFormat::kRgba32Float;
            case pb::TexelFormat::rgba32_sint:
                return core::TexelFormat::kRgba32Sint;
            case pb::TexelFormat::rgba32_uint:
                return core::TexelFormat::kRgba32Uint;
            case pb::TexelFormat::rgba8_sint:
                return core::TexelFormat::kRgba8Sint;
            case pb::TexelFormat::rgba8_snorm:
                return core::TexelFormat::kRgba8Snorm;
            case pb::TexelFormat::rgba8_uint:
                return core::TexelFormat::kRgba8Uint;
            case pb::TexelFormat::rgba8_unorm:
                return core::TexelFormat::kRgba8Unorm;
            default:
                break;
        }

        TINT_ICE() << "invalid TexelFormat: " << in;
        return core::TexelFormat::kBgra8Unorm;
    }

    core::type::SamplerKind SamplerKind(pb::SamplerKind in) {
        switch (in) {
            case pb::SamplerKind::sampler:
                return core::type::SamplerKind::kSampler;
            case pb::SamplerKind::comparison:
                return core::type::SamplerKind::kComparisonSampler;
            default:
                break;
        }

        TINT_ICE() << "invalid SamplerKind: " << in;
        return core::type::SamplerKind::kSampler;
    }

    core::InterpolationType InterpolationType(pb::InterpolationType in) {
        switch (in) {
            case pb::InterpolationType::flat:
                return core::InterpolationType::kFlat;
            case pb::InterpolationType::linear:
                return core::InterpolationType::kLinear;
            case pb::InterpolationType::perspective:
                return core::InterpolationType::kPerspective;
            default:
                break;
        }
        TINT_ICE() << "invalid InterpolationType: " << in;
        return core::InterpolationType::kFlat;
    }

    core::InterpolationSampling InterpolationSampling(pb::InterpolationSampling in) {
        switch (in) {
            case pb::InterpolationSampling::center:
                return core::InterpolationSampling::kCenter;
            case pb::InterpolationSampling::centroid:
                return core::InterpolationSampling::kCentroid;
            case pb::InterpolationSampling::sample:
                return core::InterpolationSampling::kSample;
            default:
                break;
        }
        TINT_ICE() << "invalid InterpolationSampling: " << in;
        return core::InterpolationSampling::kCenter;
    }

    core::BuiltinValue BuiltinValue(pb::BuiltinValue in) {
        switch (in) {
            case pb::BuiltinValue::point_size:
                return core::BuiltinValue::kPointSize;
            case pb::BuiltinValue::frag_depth:
                return core::BuiltinValue::kFragDepth;
            case pb::BuiltinValue::front_facing:
                return core::BuiltinValue::kFrontFacing;
            case pb::BuiltinValue::global_invocation_id:
                return core::BuiltinValue::kGlobalInvocationId;
            case pb::BuiltinValue::instance_index:
                return core::BuiltinValue::kInstanceIndex;
            case pb::BuiltinValue::local_invocation_id:
                return core::BuiltinValue::kLocalInvocationId;
            case pb::BuiltinValue::local_invocation_index:
                return core::BuiltinValue::kLocalInvocationIndex;
            case pb::BuiltinValue::num_workgroups:
                return core::BuiltinValue::kNumWorkgroups;
            case pb::BuiltinValue::position:
                return core::BuiltinValue::kPosition;
            case pb::BuiltinValue::sample_index:
                return core::BuiltinValue::kSampleIndex;
            case pb::BuiltinValue::sample_mask:
                return core::BuiltinValue::kSampleMask;
            case pb::BuiltinValue::subgroup_invocation_id:
                return core::BuiltinValue::kSubgroupInvocationId;
            case pb::BuiltinValue::subgroup_size:
                return core::BuiltinValue::kSubgroupSize;
            case pb::BuiltinValue::vertex_index:
                return core::BuiltinValue::kVertexIndex;
            case pb::BuiltinValue::workgroup_id:
                return core::BuiltinValue::kWorkgroupId;
            default:
                break;
        }
        TINT_ICE() << "invalid BuiltinValue: " << in;
        return core::BuiltinValue::kPointSize;
    }

    core::BuiltinFn BuiltinFn(pb::BuiltinFn in) {
        switch (in) {
            case pb::BuiltinFn::abs:
                return core::BuiltinFn::kAbs;
            case pb::BuiltinFn::acos:
                return core::BuiltinFn::kAcos;
            case pb::BuiltinFn::acosh:
                return core::BuiltinFn::kAcosh;
            case pb::BuiltinFn::all:
                return core::BuiltinFn::kAll;
            case pb::BuiltinFn::any:
                return core::BuiltinFn::kAny;
            case pb::BuiltinFn::array_length:
                return core::BuiltinFn::kArrayLength;
            case pb::BuiltinFn::asin:
                return core::BuiltinFn::kAsin;
            case pb::BuiltinFn::asinh:
                return core::BuiltinFn::kAsinh;
            case pb::BuiltinFn::atan:
                return core::BuiltinFn::kAtan;
            case pb::BuiltinFn::atan2:
                return core::BuiltinFn::kAtan2;
            case pb::BuiltinFn::atanh:
                return core::BuiltinFn::kAtanh;
            case pb::BuiltinFn::ceil:
                return core::BuiltinFn::kCeil;
            case pb::BuiltinFn::clamp:
                return core::BuiltinFn::kClamp;
            case pb::BuiltinFn::cos:
                return core::BuiltinFn::kCos;
            case pb::BuiltinFn::cosh:
                return core::BuiltinFn::kCosh;
            case pb::BuiltinFn::count_leading_zeros:
                return core::BuiltinFn::kCountLeadingZeros;
            case pb::BuiltinFn::count_one_bits:
                return core::BuiltinFn::kCountOneBits;
            case pb::BuiltinFn::count_trailing_zeros:
                return core::BuiltinFn::kCountTrailingZeros;
            case pb::BuiltinFn::cross:
                return core::BuiltinFn::kCross;
            case pb::BuiltinFn::degrees:
                return core::BuiltinFn::kDegrees;
            case pb::BuiltinFn::determinant:
                return core::BuiltinFn::kDeterminant;
            case pb::BuiltinFn::distance:
                return core::BuiltinFn::kDistance;
            case pb::BuiltinFn::dot:
                return core::BuiltinFn::kDot;
            case pb::BuiltinFn::dot4i8_packed:
                return core::BuiltinFn::kDot4I8Packed;
            case pb::BuiltinFn::dot4u8_packed:
                return core::BuiltinFn::kDot4U8Packed;
            case pb::BuiltinFn::dpdx:
                return core::BuiltinFn::kDpdx;
            case pb::BuiltinFn::dpdx_coarse:
                return core::BuiltinFn::kDpdxCoarse;
            case pb::BuiltinFn::dpdx_fine:
                return core::BuiltinFn::kDpdxFine;
            case pb::BuiltinFn::dpdy:
                return core::BuiltinFn::kDpdy;
            case pb::BuiltinFn::dpdy_coarse:
                return core::BuiltinFn::kDpdyCoarse;
            case pb::BuiltinFn::dpdy_fine:
                return core::BuiltinFn::kDpdyFine;
            case pb::BuiltinFn::exp:
                return core::BuiltinFn::kExp;
            case pb::BuiltinFn::exp2:
                return core::BuiltinFn::kExp2;
            case pb::BuiltinFn::extract_bits:
                return core::BuiltinFn::kExtractBits;
            case pb::BuiltinFn::face_forward:
                return core::BuiltinFn::kFaceForward;
            case pb::BuiltinFn::first_leading_bit:
                return core::BuiltinFn::kFirstLeadingBit;
            case pb::BuiltinFn::first_trailing_bit:
                return core::BuiltinFn::kFirstTrailingBit;
            case pb::BuiltinFn::floor:
                return core::BuiltinFn::kFloor;
            case pb::BuiltinFn::fma:
                return core::BuiltinFn::kFma;
            case pb::BuiltinFn::fract:
                return core::BuiltinFn::kFract;
            case pb::BuiltinFn::frexp:
                return core::BuiltinFn::kFrexp;
            case pb::BuiltinFn::fwidth:
                return core::BuiltinFn::kFwidth;
            case pb::BuiltinFn::fwidth_coarse:
                return core::BuiltinFn::kFwidthCoarse;
            case pb::BuiltinFn::fwidth_fine:
                return core::BuiltinFn::kFwidthFine;
            case pb::BuiltinFn::insert_bits:
                return core::BuiltinFn::kInsertBits;
            case pb::BuiltinFn::inverse_sqrt:
                return core::BuiltinFn::kInverseSqrt;
            case pb::BuiltinFn::ldexp:
                return core::BuiltinFn::kLdexp;
            case pb::BuiltinFn::length:
                return core::BuiltinFn::kLength;
            case pb::BuiltinFn::log:
                return core::BuiltinFn::kLog;
            case pb::BuiltinFn::log2:
                return core::BuiltinFn::kLog2;
            case pb::BuiltinFn::max:
                return core::BuiltinFn::kMax;
            case pb::BuiltinFn::min:
                return core::BuiltinFn::kMin;
            case pb::BuiltinFn::mix:
                return core::BuiltinFn::kMix;
            case pb::BuiltinFn::modf:
                return core::BuiltinFn::kModf;
            case pb::BuiltinFn::normalize:
                return core::BuiltinFn::kNormalize;
            case pb::BuiltinFn::pack2x16_float:
                return core::BuiltinFn::kPack2X16Float;
            case pb::BuiltinFn::pack2x16_snorm:
                return core::BuiltinFn::kPack2X16Snorm;
            case pb::BuiltinFn::pack2x16_unorm:
                return core::BuiltinFn::kPack2X16Unorm;
            case pb::BuiltinFn::pack4x8_snorm:
                return core::BuiltinFn::kPack4X8Snorm;
            case pb::BuiltinFn::pack4x8_unorm:
                return core::BuiltinFn::kPack4X8Unorm;
            case pb::BuiltinFn::pow:
                return core::BuiltinFn::kPow;
            case pb::BuiltinFn::quantize_to_f16:
                return core::BuiltinFn::kQuantizeToF16;
            case pb::BuiltinFn::radians:
                return core::BuiltinFn::kRadians;
            case pb::BuiltinFn::reflect:
                return core::BuiltinFn::kReflect;
            case pb::BuiltinFn::refract:
                return core::BuiltinFn::kRefract;
            case pb::BuiltinFn::reverse_bits:
                return core::BuiltinFn::kReverseBits;
            case pb::BuiltinFn::round:
                return core::BuiltinFn::kRound;
            case pb::BuiltinFn::saturate:
                return core::BuiltinFn::kSaturate;
            case pb::BuiltinFn::select:
                return core::BuiltinFn::kSelect;
            case pb::BuiltinFn::sign:
                return core::BuiltinFn::kSign;
            case pb::BuiltinFn::sin:
                return core::BuiltinFn::kSin;
            case pb::BuiltinFn::sinh:
                return core::BuiltinFn::kSinh;
            case pb::BuiltinFn::smoothstep:
                return core::BuiltinFn::kSmoothstep;
            case pb::BuiltinFn::sqrt:
                return core::BuiltinFn::kSqrt;
            case pb::BuiltinFn::step:
                return core::BuiltinFn::kStep;
            case pb::BuiltinFn::storage_barrier:
                return core::BuiltinFn::kStorageBarrier;
            case pb::BuiltinFn::tan:
                return core::BuiltinFn::kTan;
            case pb::BuiltinFn::tanh:
                return core::BuiltinFn::kTanh;
            case pb::BuiltinFn::transpose:
                return core::BuiltinFn::kTranspose;
            case pb::BuiltinFn::trunc:
                return core::BuiltinFn::kTrunc;
            case pb::BuiltinFn::unpack2x16_float:
                return core::BuiltinFn::kUnpack2X16Float;
            case pb::BuiltinFn::unpack2x16_snorm:
                return core::BuiltinFn::kUnpack2X16Snorm;
            case pb::BuiltinFn::unpack2x16_unorm:
                return core::BuiltinFn::kUnpack2X16Unorm;
            case pb::BuiltinFn::unpack4x8_snorm:
                return core::BuiltinFn::kUnpack4X8Snorm;
            case pb::BuiltinFn::unpack4x8_unorm:
                return core::BuiltinFn::kUnpack4X8Unorm;
            case pb::BuiltinFn::workgroup_barrier:
                return core::BuiltinFn::kWorkgroupBarrier;
            case pb::BuiltinFn::texture_barrier:
                return core::BuiltinFn::kTextureBarrier;
            case pb::BuiltinFn::texture_dimensions:
                return core::BuiltinFn::kTextureDimensions;
            case pb::BuiltinFn::texture_gather:
                return core::BuiltinFn::kTextureGather;
            case pb::BuiltinFn::texture_gather_compare:
                return core::BuiltinFn::kTextureGatherCompare;
            case pb::BuiltinFn::texture_num_layers:
                return core::BuiltinFn::kTextureNumLayers;
            case pb::BuiltinFn::texture_num_levels:
                return core::BuiltinFn::kTextureNumLevels;
            case pb::BuiltinFn::texture_num_samples:
                return core::BuiltinFn::kTextureNumSamples;
            case pb::BuiltinFn::texture_sample:
                return core::BuiltinFn::kTextureSample;
            case pb::BuiltinFn::texture_sample_bias:
                return core::BuiltinFn::kTextureSampleBias;
            case pb::BuiltinFn::texture_sample_compare:
                return core::BuiltinFn::kTextureSampleCompare;
            case pb::BuiltinFn::texture_sample_compare_level:
                return core::BuiltinFn::kTextureSampleCompareLevel;
            case pb::BuiltinFn::texture_sample_grad:
                return core::BuiltinFn::kTextureSampleGrad;
            case pb::BuiltinFn::texture_sample_level:
                return core::BuiltinFn::kTextureSampleLevel;
            case pb::BuiltinFn::texture_sample_base_clamp_to_edge:
                return core::BuiltinFn::kTextureSampleBaseClampToEdge;
            case pb::BuiltinFn::texture_store:
                return core::BuiltinFn::kTextureStore;
            case pb::BuiltinFn::texture_load:
                return core::BuiltinFn::kTextureLoad;
            case pb::BuiltinFn::atomic_load:
                return core::BuiltinFn::kAtomicLoad;
            case pb::BuiltinFn::atomic_store:
                return core::BuiltinFn::kAtomicStore;
            case pb::BuiltinFn::atomic_add:
                return core::BuiltinFn::kAtomicAdd;
            case pb::BuiltinFn::atomic_sub:
                return core::BuiltinFn::kAtomicSub;
            case pb::BuiltinFn::atomic_max:
                return core::BuiltinFn::kAtomicMax;
            case pb::BuiltinFn::atomic_min:
                return core::BuiltinFn::kAtomicMin;
            case pb::BuiltinFn::atomic_and:
                return core::BuiltinFn::kAtomicAnd;
            case pb::BuiltinFn::atomic_or:
                return core::BuiltinFn::kAtomicOr;
            case pb::BuiltinFn::atomic_xor:
                return core::BuiltinFn::kAtomicXor;
            case pb::BuiltinFn::atomic_exchange:
                return core::BuiltinFn::kAtomicExchange;
            case pb::BuiltinFn::atomic_compare_exchange_weak:
                return core::BuiltinFn::kAtomicCompareExchangeWeak;
            case pb::BuiltinFn::subgroup_ballot:
                return core::BuiltinFn::kSubgroupBallot;
            case pb::BuiltinFn::subgroup_broadcast:
                return core::BuiltinFn::kSubgroupBroadcast;
            default:
                break;
        }
        TINT_ICE() << "invalid BuiltinFn: " << in;
        return core::BuiltinFn::kAbs;
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
