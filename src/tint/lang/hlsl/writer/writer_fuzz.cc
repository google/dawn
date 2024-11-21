// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/cmd/fuzz/ir/fuzz.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/hlsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/hlsl/writer/writer.h"

namespace tint::hlsl::writer {
namespace {

Options GenerateOptions(core::ir::Module& module, Options::Compiler compiler) {
    Options options;
    options.disable_robustness = false;
    options.disable_workgroup_init = false;
    options.truncate_interstage_variables = false;
    options.polyfill_reflect_vec2_f32 = false;
    options.polyfill_dot_4x8_packed = false;
    options.disable_polyfill_integer_div_mod = false;
    options.polyfill_pack_unpack_4x8 = false;
    options.interstage_locations = {};
    options.root_constant_binding_point = {};
    options.pixel_local = {};

    options.compiler = compiler;
    options.bindings = GenerateBindings(module);

    options.array_length_from_uniform.ubo_binding = {30, 0};

    // Add array_length_from_uniform entries for all storage buffers with runtime sized arrays.
    std::unordered_set<tint::BindingPoint> storage_bindings;
    for (auto* inst : *module.root_block) {
        auto* var = inst->As<core::ir::Var>();
        if (!var->Result(0)->Type()->UnwrapPtr()->HasFixedFootprint()) {
            if (auto bp = var->BindingPoint()) {
                if (storage_bindings.insert(bp.value()).second) {
                    options.array_length_from_uniform.bindpoint_to_size_index.emplace(
                        bp.value(), static_cast<uint32_t>(storage_bindings.size() - 1));
                }
            }
        }
    }

    return options;
}

bool CanRun(const core::ir::Module& module) {
    // Check for unsupported module-scope variable address spaces and types.
    for (auto* inst : *module.root_block) {
        auto* var = inst->As<core::ir::Var>();
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kPushConstant) {
            return false;
        }
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            return false;
        }
        if (ptr->StoreType()->Is<core::type::InputAttachment>()) {
            return false;
        }
    }
    return true;
}

void IRFuzzerDXC(core::ir::Module& module) {
    // TODO(377391551): Enable fuzzing of options.
    auto options = GenerateOptions(module, Options::Compiler::kDXC);
    if (!CanRun(module)) {
        return;
    }
    [[maybe_unused]] auto output = Generate(module, options);
    // TODO(42251292): Fuzz DXC with HLSL output
}

void IRFuzzerFXC(core::ir::Module& module) {
    // TODO(377391551): Enable fuzzing of options.
    auto options = GenerateOptions(module, Options::Compiler::kFXC);
    if (!CanRun(module)) {
        return;
    }
    [[maybe_unused]] auto output = Generate(module, options);
    // TODO(42251292): Fuzz DXC with HLSL output
}

}  // namespace
}  // namespace tint::hlsl::writer

TINT_IR_MODULE_FUZZER(tint::hlsl::writer::IRFuzzerDXC, tint::core::ir::Capabilities{});
TINT_IR_MODULE_FUZZER(tint::hlsl::writer::IRFuzzerFXC, tint::core::ir::Capabilities{});
