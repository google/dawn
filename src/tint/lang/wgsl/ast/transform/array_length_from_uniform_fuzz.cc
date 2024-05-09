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
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/wgsl/ast/let.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/transform/array_length_from_uniform.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::ast::transform {
namespace {

bool CanRun(const Program& program, const ArrayLengthFromUniform::Config& config) {
    // The ArrayLengthFromUniform depends on SimplifyPointers, and cannot handle pointers to arrays
    // via lets (directly or via struct pointers). Reject all shaders with pointer lets.
    for (auto* node : program.ASTNodes().Objects()) {
        if (auto* let = node->As<ast::Let>()) {
            if (auto* sem = program.Sem().Get(let)) {
                if (auto* ptr = sem->Type()->As<core::type::Pointer>()) {
                    return false;
                }
            }
        }
    }

    for (auto& global : program.AST().GlobalVariables()) {
        if (auto* sem = program.Sem().Get<sem::GlobalVariable>(global)) {
            if (sem->Attributes().binding_point == config.ubo_binding) {
                // Might cause binding point collision
                return false;
            }
        }
    }
    return true;
}

void ArrayLengthFromUniformFuzzer(const Program& program,
                                  const ArrayLengthFromUniform::Config& config) {
    if (!CanRun(program, config)) {
        return;
    }

    DataMap inputs;
    inputs.Add<ArrayLengthFromUniform::Config>(std::move(config));

    DataMap outputs;
    if (auto result = ArrayLengthFromUniform{}.Apply(program, inputs, outputs)) {
        if (!result->IsValid()) {
            TINT_ICE() << "ArrayLengthFromUniform returned invalid program:\n"
                       << Program::printer(*result) << "\n"
                       << result->Diagnostics();
        }
    }
}

}  // namespace
}  // namespace tint::ast::transform

TINT_WGSL_PROGRAM_FUZZER(tint::ast::transform::ArrayLengthFromUniformFuzzer);
