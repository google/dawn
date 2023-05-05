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

#include "src/tint/writer/spirv/generator_impl_ir.h"

#include "spirv/unified1/spirv.h"
#include "src/tint/ir/module.h"
#include "src/tint/switch.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/type.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/void.h"
#include "src/tint/writer/spirv/module.h"

namespace tint::writer::spirv {

GeneratorImplIr::GeneratorImplIr(const ir::Module* module, bool zero_init_workgroup_mem)
    : ir_(module), zero_init_workgroup_memory_(zero_init_workgroup_mem) {}

bool GeneratorImplIr::Generate() {
    // TODO(crbug.com/tint/1906): Check supported extensions.

    module_.PushCapability(SpvCapabilityShader);
    module_.PushMemoryModel(spv::Op::OpMemoryModel, {U32Operand(SpvAddressingModelLogical),
                                                     U32Operand(SpvMemoryModelGLSL450)});

    // TODO(crbug.com/tint/1906): Emit extensions.

    // TODO(crbug.com/tint/1906): Emit variables.
    (void)zero_init_workgroup_memory_;

    // Emit functions.
    for (auto* func : ir_->functions) {
        EmitFunction(func);
    }

    // Serialize the module into binary SPIR-V.
    writer_.WriteHeader(module_.IdBound());
    writer_.WriteModule(&module_);

    return true;
}

uint32_t GeneratorImplIr::Type(const type::Type* ty) {
    return types_.GetOrCreate(ty, [&]() {
        auto id = module_.NextId();
        Switch(
            ty,  //
            [&](const type::Void*) { module_.PushType(spv::Op::OpTypeVoid, {id}); },
            [&](const type::Bool*) { module_.PushType(spv::Op::OpTypeBool, {id}); },
            [&](const type::I32*) {
                module_.PushType(spv::Op::OpTypeInt, {id, 32u, 1u});
            },
            [&](const type::U32*) {
                module_.PushType(spv::Op::OpTypeInt, {id, 32u, 0u});
            },
            [&](const type::F32*) {
                module_.PushType(spv::Op::OpTypeFloat, {id, 32u});
            },
            [&](const type::F16*) {
                module_.PushType(spv::Op::OpTypeFloat, {id, 16u});
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_) << "unhandled type: " << ty->FriendlyName();
            });
        return id;
    });
}

void GeneratorImplIr::EmitFunction(const ir::Function* func) {
    // Make an ID for the function.
    auto id = module_.NextId();

    // Emit the function name.
    module_.PushDebug(spv::Op::OpName, {id, Operand(func->name.Name())});

    // TODO(jrprice): Emit OpEntryPoint and OpExecutionMode declarations if needed.

    // Get the ID for the return type.
    auto return_type_id = Type(func->return_type);

    // Get the ID for the function type (creating it if needed).
    // TODO(jrprice): Add the parameter types when they are supported in the IR.
    FunctionType function_type{return_type_id, {}};
    auto function_type_id = function_types_.GetOrCreate(function_type, [&]() {
        auto func_ty_id = module_.NextId();
        OperandList operands = {func_ty_id, return_type_id};
        operands.insert(operands.end(), function_type.param_type_ids.begin(),
                        function_type.param_type_ids.end());
        module_.PushType(spv::Op::OpTypeFunction, operands);
        return func_ty_id;
    });

    // Declare the function.
    auto decl =
        Instruction{spv::Op::OpFunction,
                    {return_type_id, id, U32Operand(SpvFunctionControlMaskNone), function_type_id}};

    // Create a function that we will add instructions to.
    // TODO(jrprice): Add the parameter declarations when they are supported in the IR.
    auto entry_block = module_.NextId();
    Function current_function_(decl, entry_block, {});

    // TODO(jrprice): Emit the body of the function.

    // TODO(jrprice): Remove this when we start emitting OpReturn for branches to the terminator.
    current_function_.push_inst(spv::Op::OpReturn, {});

    // Add the function to the module.
    module_.PushFunction(current_function_);
}

}  // namespace tint::writer::spirv
