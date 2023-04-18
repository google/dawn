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

#include "src/tint/ast/identifier.h"
#include "src/tint/ast/module.h"
#include "src/tint/bench/benchmark.h"

namespace tint::writer::glsl {
namespace {

void GenerateGLSL(benchmark::State& state, std::string input_name) {
    auto res = bench::LoadProgram(input_name);
    if (auto err = std::get_if<bench::Error>(&res)) {
        state.SkipWithError(err->msg.c_str());
        return;
    }
    auto& program = std::get<bench::ProgramAndFile>(res).program;
    std::vector<std::string> entry_points;
    for (auto& fn : program.AST().Functions()) {
        if (fn->IsEntryPoint()) {
            entry_points.emplace_back(fn->name->symbol.Name());
        }
    }

    for (auto _ : state) {
        for (auto& ep : entry_points) {
            auto res = Generate(&program, {}, ep);
            if (!res.error.empty()) {
                state.SkipWithError(res.error.c_str());
            }
        }
    }
}

TINT_BENCHMARK_WGSL_PROGRAMS(GenerateGLSL);

}  // namespace
}  // namespace tint::writer::glsl
