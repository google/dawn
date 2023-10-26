// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/msl/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/msl/writer/printer/printer.h"
#include "src/tint/lang/msl/writer/raise/raise.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/lower/lower.h"
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
        auto result = Print(ir);
        if (!result) {
            return result.Failure();
        }
        output.msl = result.Get();
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
