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
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/glsl/writer/writer.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/inspector/inspector.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::glsl::writer {
namespace {

bool CanRun(const tint::Program& program, const Options& options) {
    if (program.AST().HasOverrides()) {
        return false;  // Overrides are not supported.
    }

    // Excessive values can cause OOM / timeouts in the PadStructs transform.
    static constexpr uint32_t kMaxOffset = 0x1000;

    if (options.first_instance_offset && options.first_instance_offset > kMaxOffset) {
        return false;
    }

    if (options.first_vertex_offset && options.first_vertex_offset > kMaxOffset) {
        return false;
    }

    if (options.depth_range_offsets) {
        if (options.depth_range_offsets->max > kMaxOffset ||
            options.depth_range_offsets->min > kMaxOffset) {
            return false;
        }
    }

    // Make sure that every texture variable is in the texture_builtins_from_uniform binding list,
    // otherwise TextureBuiltinsFromUniform will fail.
    for (auto* global : program.AST().GlobalVariables()) {
        auto* sem = program.Sem().Get<sem::GlobalVariable>(global);
        if (!sem || !sem->Type()->UnwrapRef()->Is<core::type::Texture>()) {
            continue;
        }

        bool found = false;
        auto binding_point = sem->Attributes().binding_point;
        for (auto& bp : options.bindings.texture_builtins_from_uniform.ubo_bindingpoint_ordering) {
            if (bp == binding_point) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}

void ASTFuzzer(const tint::Program& program,
               const fuzz::wgsl::Context& context,
               const Options& options) {
    if (!CanRun(program, options)) {
        return;
    }

    auto inspector = tint::inspector::Inspector(program);
    auto entrypoints = inspector.GetEntryPoints();

    // Test all of the entry points as GLSL requires specifying which one to generate.
    for (const auto& ep : entrypoints) {
        auto res = tint::glsl::writer::Generate(program, options, ep.name);

        if (res == Success && context.options.dump) {
            std::cout << "Dumping generated GLSL:\n" << res->glsl << std::endl;
        }
    }
}

}  // namespace
}  // namespace tint::glsl::writer

TINT_WGSL_PROGRAM_FUZZER(tint::glsl::writer::ASTFuzzer);
