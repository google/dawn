// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/glsl/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/glsl/writer/printer/printer.h"
#include "src/tint/lang/glsl/writer/raise/raise.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/lower/lower.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#endif  // TINT_BUILD_WGSL_REAdDER

namespace tint::glsl::writer {

Result<Output> Generate(const Program& program,
                        const Options& options,
                        const std::string& entry_point) {
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

        // Raise from core-dialect to GLSL-dialect.
        if (auto res = raise::Raise(ir); !res) {
            return res.Failure();
        }

        // Generate the GLSL code.
        auto result = Print(ir, options.version);
        if (!result) {
            return result.Failure();
        }
        output.glsl = result.Get();
#else
        return Failure{"use_tint_ir requires building with TINT_BUILD_WGSL_READER"};
#endif
    } else {
        // Sanitize the program.
        auto sanitized_result = Sanitize(program, options, entry_point);
        if (!sanitized_result.program.IsValid()) {
            return Failure{sanitized_result.program.Diagnostics()};
        }

        // Generate the GLSL code.
        auto impl = std::make_unique<ASTPrinter>(sanitized_result.program, options.version);
        if (!impl->Generate()) {
            return Failure{impl->Diagnostics()};
        }

        output.glsl = impl->Result();
        output.needs_internal_uniform_buffer = sanitized_result.needs_internal_uniform_buffer;
        output.bindpoint_to_data = std::move(sanitized_result.bindpoint_to_data);

        // Collect the list of entry points in the sanitized program.
        for (auto* func : sanitized_result.program.AST().Functions()) {
            if (func->IsEntryPoint()) {
                auto name = func->name->symbol.Name();
                output.entry_points.push_back({name, func->PipelineStage()});
            }
        }
    }

    return output;
}

}  // namespace tint::glsl::writer
