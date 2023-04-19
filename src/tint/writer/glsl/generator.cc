// Copyright 2021 The Tint Authors.
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

#include "src/tint/writer/glsl/generator.h"

#include "src/tint/transform/binding_remapper.h"
#include "src/tint/transform/combine_samplers.h"
#include "src/tint/writer/glsl/generator_impl.h"

namespace tint::writer::glsl {

Options::Options() = default;
Options::~Options() = default;
Options::Options(const Options&) = default;

Result::Result() = default;
Result::~Result() = default;
Result::Result(const Result&) = default;

Result Generate(const Program* program, const Options& options, const std::string& entry_point) {
    Result result;
    if (!program->IsValid()) {
        result.error = "input program is not valid";
        return result;
    }

    // Sanitize the program.
    auto sanitized_result = Sanitize(program, options, entry_point);
    if (!sanitized_result.program.IsValid()) {
        result.success = false;
        result.error = sanitized_result.program.Diagnostics().str();
        return result;
    }

    // Generate the GLSL code.
    auto impl = std::make_unique<GeneratorImpl>(&sanitized_result.program, options.version);
    impl->Generate();
    result.success = impl->Diagnostics().empty();
    result.error = impl->Diagnostics().str();
    result.glsl = impl->result();

    // Collect the list of entry points in the sanitized program.
    for (auto* func : sanitized_result.program.AST().Functions()) {
        if (func->IsEntryPoint()) {
            auto name = func->name->symbol.Name();
            result.entry_points.push_back({name, func->PipelineStage()});
        }
    }

    return result;
}

}  // namespace tint::writer::glsl
