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

// GEN_BUILD:CONDITION(tint_build_wgsl_reader)

#include "src/tint/lang/spirv/writer/writer.h"

#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/lang/spirv/validate/validate.h"
#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"

namespace tint::spirv::writer {
namespace {

void ASTPrinterFuzzer(const tint::Program& program, const Options& options) {
    auto transformed = tint::wgsl::ApplySubstituteOverrides(program);
    auto& no_overrides = transformed ? transformed.value() : program;
    if (!no_overrides.IsValid()) {
        return;
    }
    auto output = Generate(no_overrides, options);
    if (!output) {
        return;
    }
    auto& spirv = output->spirv;
    if (auto res = validate::Validate(Slice(spirv.data(), spirv.size())); !res) {
        TINT_ICE() << "Output of SPIR-V writer failed to validate with SPIR-V Tools\n"
                   << res.Failure();
    }
}

}  // namespace
}  // namespace tint::spirv::writer

TINT_WGSL_PROGRAM_FUZZER(tint::spirv::writer::ASTPrinterFuzzer);
