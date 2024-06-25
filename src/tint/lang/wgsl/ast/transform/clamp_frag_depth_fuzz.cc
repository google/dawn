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

#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/transform/clamp_frag_depth.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::ast::transform {
namespace {

bool CanRun(const Program& program, const ClampFragDepth::Config& config) {
    if (config.offsets) {
        if (config.offsets->min >= config.offsets->max) {
            return false;  // member offset collision / non-ascending
        }
        if ((config.offsets->min & 3) != 0 || (config.offsets->max & 3) != 0) {
            return false;  // Offsets need 4-byte alignment.
        }
    }

    // ClampFragDepth assumes that AddBlockAttribute has run, which makes sure that all push
    // constant variables are structures.
    for (auto& global : program.AST().GlobalVariables()) {
        if (auto* sem = program.Sem().Get<sem::GlobalVariable>(global)) {
            if (sem->AddressSpace() == core::AddressSpace::kPushConstant) {
                if (!sem->Type()->UnwrapRef()->Is<core::type::Struct>()) {
                    return false;
                }
            }
        }
    }

    return true;
}

void ClampFragDepthFuzzer(const Program& program, const ClampFragDepth::Config& config) {
    if (!CanRun(program, config)) {
        return;
    }

    DataMap inputs;
    inputs.Add<ClampFragDepth::Config>(std::move(config));

    DataMap outputs;
    if (auto result = ClampFragDepth{}.Apply(program, inputs, outputs)) {
        if (!result->IsValid()) {
            TINT_ICE() << "ClampFragDepth returned invalid program:\n"
                       << Program::printer(*result) << "\n"
                       << result->Diagnostics();
        }
    }
}

}  // namespace
}  // namespace tint::ast::transform

TINT_WGSL_PROGRAM_FUZZER(tint::ast::transform::ClampFragDepthFuzzer);
