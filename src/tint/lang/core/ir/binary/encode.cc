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

#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
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

    void Encode() {
        Vector<pb::Function*, 8> fns_out;
        for (auto& fn_in : mod_in_.functions) {
            uint32_t id = static_cast<uint32_t>(fns_out.Length() + 1);
            fns_out.Push(mod_out_.add_functions());
            functions_.Add(fn_in, id);
        }
        for (size_t i = 0, n = mod_in_.functions.Length(); i < n; i++) {
            Function(fns_out[i], mod_in_.functions[i]);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////
    void Function(pb::Function* fn_out, const ir::Function* fn_in) {
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
            fn_out->add_parameters(FunctionParam(param_in));
        }
        fn_out->set_block(Block(fn_in->Block()));
    }

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
                Instruction(block_out.add_instructions(), inst);
            }
            return static_cast<uint32_t>(blocks_.Count());
        });
    }

    ////////////////////////////////////////////////////////////////////////////
    // Instructions
    ////////////////////////////////////////////////////////////////////////////
    void Instruction(pb::Instruction* inst_out, const ir::Instruction* inst_in) {
        auto kind = Switch(
            inst_in,                                                         //
            [&](const ir::Return*) { return pb::InstructionKind::Return; },  //
            TINT_ICE_ON_NO_MATCH);
        inst_out->set_kind(kind);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////
    uint32_t Type(const core::type::Type* type) {
        if (type == nullptr) {
            return 0;
        }
        return types_.GetOrCreate(type, [&]() -> uint32_t {
            auto basic = tint::Switch<pb::BasicType>(
                type,  //
                [&](const core::type::Void*) { return pb::BasicType::void_; },
                [&](const core::type::Bool*) { return pb::BasicType::bool_; },
                [&](const core::type::I32*) { return pb::BasicType::i32; },
                [&](const core::type::U32*) { return pb::BasicType::u32; },
                [&](const core::type::F32*) { return pb::BasicType::f32; },  //
                TINT_ICE_ON_NO_MATCH);
            mod_out_.add_types()->set_basic(basic);
            return static_cast<uint32_t>(types_.Count());
        });
    }

    ////////////////////////////////////////////////////////////////////////////
    // Values
    ////////////////////////////////////////////////////////////////////////////
    // uint32_t Value(const ir::Value* value) {
    //     return Switch(value,  //
    //                   [&](const ir::FunctionParam* p) { return FunctionParam(p); });
    // }

    uint32_t FunctionParam(const ir::FunctionParam* param) {
        return values_.GetOrCreate(param, [&] {
            auto& val_out = *mod_out_.add_values();
            val_out.set_kind(pb::ValueKind::function_parameter);
            val_out.set_type(Type(param->Type()));
            if (auto name = mod_in_.NameOf(param); name.IsValid()) {
                val_out.set_name(name.Name());
            }
            return static_cast<uint32_t>(values_.Count());
        });
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
