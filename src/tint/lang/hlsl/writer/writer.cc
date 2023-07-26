// Copyright 2020 The Tint Authors.
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

#include "src/tint/lang/hlsl/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/hlsl/writer/ast_printer/ast_printer.h"

namespace tint::hlsl::writer {

Result Generate(const Program* program, const Options& options) {
    Result result;
    if (!program->IsValid()) {
        result.error = "input program is not valid";
        return result;
    }

    // Sanitize the program.
    auto sanitized_result = Sanitize(program, options);
    if (!sanitized_result.program.IsValid()) {
        result.success = false;
        result.error = sanitized_result.program.Diagnostics().str();
        return result;
    }

    // Generate the HLSL code.
    auto impl = std::make_unique<ASTPrinter>(&sanitized_result.program);
    result.success = impl->Generate();
    result.error = impl->Diagnostics().str();
    result.hlsl = impl->Result();

    // Collect the list of entry points in the sanitized program.
    for (auto* func : sanitized_result.program.AST().Functions()) {
        if (func->IsEntryPoint()) {
            auto name = func->name->symbol.Name();
            result.entry_points.push_back({name, func->PipelineStage()});
        }
    }

    result.used_array_length_from_uniform_indices =
        std::move(sanitized_result.used_array_length_from_uniform_indices);

    return result;
}

}  // namespace tint::hlsl::writer
