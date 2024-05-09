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

#include "src/tint/api/common/binding_point.h"
#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/lang/core/access.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::ast::transform {
namespace {

bool CanRun(const Program& program, const BindingRemapper::Remappings& remappings) {
    if (!remappings.access_controls.empty()) {
        // Changing access is likely to cause WGSL validation failures. Just skip these for now.
        return false;
    }

    if (!remappings.allow_collisions) {
        Hashset<BindingPoint, 8> binding_points;
        for (auto* global : program.AST().GlobalVariables()) {
            if (auto* sem = program.Sem().Get<sem::GlobalVariable>(global)) {
                if (auto binding_point = sem->Attributes().binding_point) {
                    binding_points.Add(binding_point.value());
                }
            }
        }
        Hashset<BindingPoint, 8> new_binding_points;
        for (auto& remapping : remappings.binding_points) {
            if (binding_points.Remove(remapping.first)) {
                if (!new_binding_points.Add(remapping.second)) {
                    return false;  // Binding collision
                }
            }
        }
        for (auto& binding_point : new_binding_points) {
            if (!binding_points.Add(binding_point)) {
                return false;  // Binding collision
            }
        }
    }
    return true;
}

void BindingRemapperFuzzer(const Program& program, const BindingRemapper::Remappings& remappings) {
    if (!CanRun(program, remappings)) {
        return;
    }

    DataMap inputs;
    inputs.Add<BindingRemapper::Remappings>(std::move(remappings));

    DataMap outputs;
    if (auto result = BindingRemapper{}.Apply(program, inputs, outputs)) {
        if (!result->IsValid()) {
            TINT_ICE() << "BindingRemapper returned invalid program:\n"
                       << Program::printer(*result) << "\n"
                       << result->Diagnostics();
        }
    }
}

}  // namespace
}  // namespace tint::ast::transform

TINT_WGSL_PROGRAM_FUZZER(tint::ast::transform::BindingRemapperFuzzer);
