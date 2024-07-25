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

#include <iostream>

#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/lang/msl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/msl/writer/writer.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::msl::writer {
namespace {

bool CanRun(const Program& program) {
    if (program.AST().HasOverrides()) {
        // MSL writer assumes the SubstituteOverride and SingleEntryPoint transforms have been run.
        return false;
    }

    // Check for push constants, which the MSL writer does not support.
    for (auto* global : program.AST().GlobalVariables()) {
        if (program.Sem().Get(global)->AddressSpace() == core::AddressSpace::kPushConstant) {
            return false;
        }
    }

    return true;
}

void ASTFuzzer(const tint::Program& program, const fuzz::wgsl::Context& context, Options options) {
    if (!CanRun(program)) {
        return;
    }

    options.bindings = GenerateBindings(program);

    auto res = tint::msl::writer::Generate(program, options);

    if (res == Success && context.options.dump) {
        std::cout << "Dumping generated MSL:\n" << res->msl << std::endl;
    }
}

}  // namespace
}  // namespace tint::msl::writer

TINT_WGSL_PROGRAM_FUZZER(tint::msl::writer::ASTFuzzer);
