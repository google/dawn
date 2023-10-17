// Copyright 2022 The Dawn & Tint Authors
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

#include <string>

#include "src/tint/cmd/bench/bench.h"
#include "src/tint/lang/msl/writer/writer.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::msl::writer {
namespace {

void GenerateMSL(benchmark::State& state, std::string input_name) {
    auto res = bench::LoadProgram(input_name);
    if (!res) {
        state.SkipWithError(res.Failure().reason.str());
        return;
    }
    auto& program = res->program;

    tint::msl::writer::Options gen_options = {};
    gen_options.array_length_from_uniform.ubo_binding = tint::BindingPoint{0, 30};
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 0},
                                                                          0);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 1},
                                                                          1);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 2},
                                                                          2);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 3},
                                                                          3);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 4},
                                                                          4);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 5},
                                                                          5);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 6},
                                                                          6);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 7},
                                                                          7);

    uint32_t next_binding_point = 0;
    for (auto* var : program.AST().GlobalVariables()) {
        if (auto* var_sem = program.Sem().Get(var)->As<sem::GlobalVariable>()) {
            if (auto bp = var_sem->BindingPoint()) {
                gen_options.binding_remapper_options.binding_points[*bp] = BindingPoint{
                    0,                     // group
                    next_binding_point++,  // binding
                };
            }
        }
    }
    for (auto _ : state) {
        auto gen_res = Generate(program, gen_options);
        if (!gen_res) {
            state.SkipWithError(gen_res.Failure().reason.str());
        }
    }
}

TINT_BENCHMARK_PROGRAMS(GenerateMSL);

}  // namespace
}  // namespace tint::msl::writer
