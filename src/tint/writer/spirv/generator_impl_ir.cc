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

    // TODO(crbug.com/tint/1906): Emit functions.
    (void)ir_;

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

}  // namespace tint::writer::spirv
