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
            b.ir.functions.Reserve(n);
            for (auto& fn_in : mod_in_.functions()) {
                b.ir.functions.Push(CreateFunction(fn_in));
            }
        }
        {
            const size_t n = static_cast<size_t>(mod_in_.blocks().size());
            blocks_.Reserve(n);
            for (auto& block_in : mod_in_.blocks()) {
                blocks_.Push(CreateBlock(block_in));
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
            PopulateFunction(b.ir.functions[i], mod_in_.functions()[static_cast<int>(i)]);
        }
        for (size_t i = 0, n = static_cast<size_t>(mod_in_.blocks().size()); i < n; i++) {
            PopulateBlock(blocks_[i], mod_in_.blocks()[static_cast<int>(i)]);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////
    ir::Function* CreateFunction(const pb::Function&) { return b.ir.values.Create<ir::Function>(); }

    void PopulateFunction(ir::Function* fn_out, const pb::Function& fn_in) {
        if (!fn_in.name().empty()) {
            b.ir.SetName(fn_out, fn_in.name());
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

    ir::Function* Function(uint32_t id) { return id > 0 ? b.ir.functions[id - 1] : nullptr; }

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
        switch (inst_in.kind()) {
            case pb::InstructionKind::Discard:
                inst_out = b.ir.instructions.Create<ir::Discard>();
                break;
            case pb::InstructionKind::Return:
                inst_out = b.ir.instructions.Create<ir::Return>();
                break;
            case pb::InstructionKind::Let:
                inst_out = b.ir.instructions.Create<ir::Let>();
                break;
            default:
                TINT_UNIMPLEMENTED() << inst_in.kind();
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

    ////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////
    const type::Type* CreateType(const pb::TypeDecl type_in) {
        switch (type_in.kind_case()) {
            case pb::TypeDecl::KindCase::kBasic:
                switch (type_in.basic()) {
                    case pb::BasicType::void_:
                        return mod_out_.Types().Get<void>();
                    case pb::BasicType::bool_:
                        return mod_out_.Types().Get<bool>();
                    case pb::BasicType::i32:
                        return mod_out_.Types().Get<i32>();
                    case pb::BasicType::u32:
                        return mod_out_.Types().Get<u32>();
                    case pb::BasicType::f32:
                        return mod_out_.Types().Get<f32>();
                    case pb::BasicType::f16:
                        return mod_out_.Types().Get<f16>();
                    default:
                        TINT_ICE() << "invalid BasicType: " << type_in.basic();
                        return nullptr;
                }
            case pb::TypeDecl::KindCase::kVector:
            case pb::TypeDecl::KindCase::kMatrix:
            case pb::TypeDecl::KindCase::kArray:
            case pb::TypeDecl::KindCase::kAtomic:
                TINT_UNIMPLEMENTED() << type_in.kind_case();
                return nullptr;

            case pb::TypeDecl::KindCase::KIND_NOT_SET:
                break;
        }
        TINT_ICE() << "invalid TypeDecl.kind";
        return nullptr;
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
                    b.ir.SetName(value_out, res_in.name());
                }
                break;
            }
            case pb::Value::KindCase::kFunctionParameter: {
                auto& param_in = value_in.function_parameter();
                auto* type = Type(param_in.type());
                value_out = b.FunctionParam(type);
                if (param_in.has_name()) {
                    b.ir.SetName(value_out, param_in.name());
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
            case pb::ConstantValue::KindCase::kScalar: {
                return CreateScalarValue(value_in.scalar());
            }
            case pb::ConstantValue::KindCase::kSplat: {
                auto& splat_in = value_in.splat();
                auto* type = Type(splat_in.type());
                auto* elem = ConstantValue(splat_in.elements());
                return b.ir.constant_values.Splat(type, elem, splat_in.count());
            }
            default:
                TINT_ICE() << "invalid ConstantValue.kind: " << value_in.kind_case();
                return nullptr;
        }
    }

    const core::constant::Value* CreateScalarValue(const pb::ConstantValueScalar& value_in) {
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

    const core::constant::Value* ConstantValue(uint32_t id) {
        return id > 0 ? constant_values_[id - 1] : nullptr;
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
