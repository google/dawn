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
    Hashmap<uint32_t, ir::Block*, 32> blocks_{};
    Hashmap<uint32_t, const type::Type*, 32> types_{};
    Hashmap<uint32_t, ir::Value*, 32> values_{};
    Builder b{mod_out_};

    void Decode() {
        // Build all the functions in a separate pass, before we decode them.
        // This allows for forward references, while preserving function declaration order.
        for (size_t i = 0, n = static_cast<size_t>(mod_in_.functions().size()); i < n; i++) {
            b.ir.functions.Push(b.ir.values.Create<ir::Function>());
        }
        for (size_t i = 0, n = static_cast<size_t>(mod_in_.functions().size()); i < n; i++) {
            Function(b.ir.functions[i], mod_in_.functions()[static_cast<int>(i)]);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////
    void Function(ir::Function* fn_out, const pb::Function& fn_in) {
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
    ir::Block* Block(uint32_t id) {
        if (id == 0) {
            return nullptr;
        }
        return blocks_.GetOrCreate(id, [&] {
            auto& block_in = mod_in_.blocks().at(static_cast<int>(id) - 1);
            auto* block_out = b.Block();
            for (auto& inst : block_in.instructions()) {
                block_out->Append(Instruction(inst));
            }
            return block_out;
        });
    }

    ////////////////////////////////////////////////////////////////////////////
    // Instructions
    ////////////////////////////////////////////////////////////////////////////
    ir::Instruction* Instruction(const pb::Instruction& inst_in) {
        ir::Instruction* inst_out = nullptr;
        switch (inst_in.kind()) {
            case pb::InstructionKind::Return:
                inst_out = b.ir.instructions.Create<ir::Return>();
                break;
            default:
                TINT_UNIMPLEMENTED() << inst_in.kind();
                break;
        }
        TINT_ASSERT_OR_RETURN_VALUE(inst_out, nullptr);

        return inst_out;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////
    const type::Type* Type(uint32_t id) {
        if (id == 0) {
            return nullptr;
        }
        return types_.GetOrCreate(id, [&]() -> const type::Type* {
            auto& ty_in = mod_in_.types().at(static_cast<int>(id) - 1);
            switch (ty_in.kind_case()) {
                case pb::TypeDecl::KindCase::kBasic:
                    switch (ty_in.basic()) {
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
                        default:
                            TINT_ICE() << "invalid BasicType: " << ty_in.basic();
                            return nullptr;
                    }
                case pb::TypeDecl::KindCase::kVector:
                case pb::TypeDecl::KindCase::kMatrix:
                case pb::TypeDecl::KindCase::kArray:
                case pb::TypeDecl::KindCase::kAtomic:
                    TINT_UNIMPLEMENTED() << ty_in.kind_case();
                    return nullptr;

                case pb::TypeDecl::KindCase::KIND_NOT_SET:
                    break;
            }
            TINT_ICE() << "invalid TypeDecl.kind";
            return nullptr;
        });
    }

    ////////////////////////////////////////////////////////////////////////////
    // Values
    ////////////////////////////////////////////////////////////////////////////
    ir::Value* Value(uint32_t id) {
        if (id == 0) {
            return nullptr;
        }
        return values_.GetOrCreate(id, [&]() -> ir::Value* {
            auto& val_in = mod_in_.values().at(static_cast<int>(id) - 1);
            auto* type = Type(val_in.type());
            ir::Value* val_out = nullptr;
            switch (val_in.kind()) {
                case pb::ValueKind::instruction_result:
                    val_out = b.InstructionResult(type);
                    break;
                case pb::ValueKind::function_parameter:
                    val_out = b.FunctionParam(type);
                    break;
                default:
                    TINT_ICE() << "invalid TypeDecl.kind";
                    return nullptr;
            }
            if (val_in.has_name()) {
                mod_out_.SetName(val_out, val_in.name());
            }
            return val_out;
        });
    }

    template <typename T>
    T* ValueAs(uint32_t id) {
        auto* value = Value(id);
        if (auto cast = value->As<T>(); TINT_LIKELY(cast)) {
            return cast;
        }
        TINT_ICE() << "Value " << id << " is " << value->TypeInfo().name << " expected "
                   << TypeInfo::Of<T>().name;
        return nullptr;
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
