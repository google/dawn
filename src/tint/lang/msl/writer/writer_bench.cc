// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "src/tint/bench/benchmark.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::msl::writer {
namespace {

void GenerateMSL(benchmark::State& state, std::string input_name) {
    auto res = bench::LoadProgram(input_name);
    if (auto err = std::get_if<bench::Error>(&res)) {
        state.SkipWithError(err->msg.c_str());
        return;
    }
    auto& program = std::get<bench::ProgramAndFile>(res).program;

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
        auto res = Generate(&program, gen_options);
        if (!res) {
            state.SkipWithError(res.Failure().c_str());
        }
    }
}

TINT_BENCHMARK_PROGRAMS(GenerateMSL);

}  // namespace
}  // namespace tint::msl::writer
