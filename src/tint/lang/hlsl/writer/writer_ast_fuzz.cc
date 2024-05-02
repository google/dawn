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

// GEN_BUILD:CONDITION(tint_build_wgsl_reader)

#include <string>
#include <unordered_map>

#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/lang/hlsl/validate/validate.h"
#include "src/tint/lang/hlsl/writer/writer.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/utils/command/command.h"

namespace tint::hlsl::writer {
namespace {

void ASTFuzzer(const tint::Program& program,
               const fuzz::wgsl::Options& fuzz_options,
               Options options) {
    if (program.AST().HasOverrides()) {
        return;
    }

    auto res = tint::hlsl::writer::Generate(program, options);
    if (res == Success) {
        const char* dxc_path = validate::kDxcDLLName;
        bool must_validate = false;
        if (!fuzz_options.dxc.empty()) {
            must_validate = true;
            dxc_path = fuzz_options.dxc.c_str();
        }

        auto dxc = tint::Command::LookPath(dxc_path);
        if (dxc.Found()) {
            uint32_t hlsl_shader_model = 60;
            bool require_16bit_types = false;
            auto enable_list = program.AST().Enables();
            for (auto* enable : enable_list) {
                if (enable->HasExtension(tint::wgsl::Extension::kF16)) {
                    hlsl_shader_model = 62;
                    require_16bit_types = true;
                    break;
                }
            }

            auto validate_res = validate::ValidateUsingDXC(dxc.Path(), res->hlsl, res->entry_points,
                                                           require_16bit_types, hlsl_shader_model);

            if (must_validate && validate_res.failed) {
                TINT_ICE() << "DXC was expected to succeed, but failed: " << validate_res.output;
            }

        } else if (must_validate) {
            TINT_ICE() << "DXC path was explicitly specified, but was not found: " << dxc_path;
        }
    }
}

}  // namespace
}  // namespace tint::hlsl::writer

TINT_WGSL_PROGRAM_FUZZER(tint::hlsl::writer::ASTFuzzer);
