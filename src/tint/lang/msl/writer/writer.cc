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

#include "src/tint/lang/msl/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/msl/writer/ast_printer/ast_printer.h"

#if TINT_BUILD_IR
#include "src/tint/lang/core/ir/from_program.h"        // nogncheck
#include "src/tint/lang/msl/writer/printer/printer.h"  // nogncheck
#endif                                                 // TINT_BUILD_IR

namespace tint::msl::writer {

Result Generate(const Program* program, const Options& options) {
    Result result;
    if (!program->IsValid()) {
        result.error = "input program is not valid";
        return result;
    }

#if TINT_BUILD_IR
    if (options.use_tint_ir) {
        // Convert the AST program to an IR module.
        auto converted = ir::FromProgram(program);
        if (!converted) {
            result.error = "IR converter: " + converted.Failure();
            return result;
        }

        // Generate the MSL code.
        auto ir = converted.Move();
        auto impl = std::make_unique<Printer>(&ir);
        result.success = impl->Generate();
        result.error = impl->Diagnostics().str();
        result.msl = impl->Result();
    } else  // NOLINT(readability/braces)
#endif
    {
        // Sanitize the program.
        auto sanitized_result = Sanitize(program, options);
        if (!sanitized_result.program.IsValid()) {
            result.success = false;
            result.error = sanitized_result.program.Diagnostics().str();
            return result;
        }
        result.needs_storage_buffer_sizes = sanitized_result.needs_storage_buffer_sizes;
        result.used_array_length_from_uniform_indices =
            std::move(sanitized_result.used_array_length_from_uniform_indices);

        // Generate the MSL code.
        auto impl = std::make_unique<ASTPrinter>(&sanitized_result.program);
        result.success = impl->Generate();
        result.error = impl->Diagnostics().str();
        result.msl = impl->Result();
        result.has_invariant_attribute = impl->HasInvariant();
        result.workgroup_allocations = impl->DynamicWorkgroupAllocations();
    }

    return result;
}

}  // namespace tint::msl::writer
