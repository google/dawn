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
#include "src/tint/lang/msl/writer/printer/printer.h"
#include "src/tint/lang/msl/writer/raise/raise.h"
#include "src/tint/lang/wgsl/reader/lower/lower.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#endif

namespace tint::msl::writer {

Result<Output> Generate(const Program& program, const Options& options) {
    if (!program.IsValid()) {
        return Failure{program.Diagnostics()};
    }

    Output output;

    if (options.use_tint_ir) {
#if TINT_BUILD_WGSL_READER
        // Convert the AST program to an IR module.
        auto converted = wgsl::reader::ProgramToIR(program);
        if (!converted) {
            return converted.Failure();
        }

        auto ir = converted.Move();

        // Lower from WGSL-dialect to core-dialect
        if (auto res = wgsl::reader::Lower(ir); !res) {
            return res.Failure();
        }

        // Raise from core-dialect to MSL-dialect.
        if (auto res = raise::Raise(ir); !res) {
            return res.Failure();
        }

        // Generate the MSL code.
        auto impl = std::make_unique<Printer>(ir);
        auto result = impl->Generate();
        if (!result) {
            return result.Failure();
        }
        output.msl = impl->Result();
#else
        return Failure{"use_tint_ir requires building with TINT_BUILD_WGSL_READER"};
#endif
    } else {
        // Sanitize the program.
        auto sanitized_result = Sanitize(program, options);
        if (!sanitized_result.program.IsValid()) {
            return Failure{sanitized_result.program.Diagnostics()};
        }
        output.needs_storage_buffer_sizes = sanitized_result.needs_storage_buffer_sizes;
        output.used_array_length_from_uniform_indices =
            std::move(sanitized_result.used_array_length_from_uniform_indices);

        // Generate the MSL code.
        auto impl = std::make_unique<ASTPrinter>(sanitized_result.program);
        if (!impl->Generate()) {
            return Failure{impl->Diagnostics()};
        }
        output.msl = impl->Result();
        output.has_invariant_attribute = impl->HasInvariant();
        output.workgroup_allocations = impl->DynamicWorkgroupAllocations();
    }

    return output;
}

}  // namespace tint::msl::writer
