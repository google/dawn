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

#include "src/tint/lang/spirv/writer/writer.h"

#include "src/tint/api/helpers/generate_bindings.h"
#include "src/tint/cmd/fuzz/ir/fuzz.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/transform/resource_binding_helper.h"
#include "src/tint/lang/spirv/validate/validate.h"
#include "src/tint/lang/spirv/writer/printer/printer.h"

namespace tint::spirv::writer {
namespace {

Result<SuccessType> IRFuzzer(core::ir::Module& module, const fuzz::ir::Context&, Options options) {
    // TODO(375388101): We cannot run the backend for every entry point in the module unless we
    // clone the whole module each time, so for now we just generate the first entry point.

    // Strip the module down to a single entry point.
    core::ir::Function* entry_point = nullptr;
    for (auto& func : module.functions) {
        if (func->IsEntryPoint()) {
            entry_point = func;
            break;
        }
    }
    std::string ep_name;
    if (entry_point) {
        ep_name = module.NameOf(entry_point).NameView();
    }
    if (ep_name.empty()) {
        // No entry point, just return success
        return Success;
    }

    auto check = CanGenerate(module, options, ep_name);
    if (check != Success) {
        return Failure{check.Failure().reason};
    }

    options.bindings = GenerateBindings(module, ep_name, false, false);
    options.resource_binding = tint::core::ir::transform::GenerateResourceBindingConfig(module);

    auto output = Generate(module, options);
    if (output != Success) {
        TINT_ICE() << "Generate() failed after CanGenerate() succeeded: "
                   << output.Failure().reason;
    }

    spv_target_env target_env = SPV_ENV_VULKAN_1_1;
    switch (options.spirv_version) {
        case SpvVersion::kSpv13:
            target_env = SPV_ENV_VULKAN_1_1;
            break;
        case SpvVersion::kSpv14:
            target_env = SPV_ENV_VULKAN_1_1_SPIRV_1_4;
            break;
        case SpvVersion::kSpv15:
            target_env = SPV_ENV_VULKAN_1_2;
            break;
        default:
            TINT_ICE() << "unsupported SPIR-V version";
    }

    auto& spirv = output->spirv;
    if (auto res = validate::Validate(Slice(spirv.data(), spirv.size()), target_env);
        res != Success) {
        TINT_ICE() << "output of SPIR-V writer failed to validate with SPIR-V Tools\n"
                   << res.Failure() << "\n\n"
                   << "IR:\n"
                   << core::ir::Disassembler(module).Plain();
    }

    return Success;
}

}  // namespace
}  // namespace tint::spirv::writer

TINT_IR_MODULE_FUZZER(tint::spirv::writer::IRFuzzer,
                      tint::core::ir::Capabilities{},
                      tint::spirv::writer::kPrinterCapabilities);
