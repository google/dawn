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

#include "src/tint/lang/spirv/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/spirv/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/spirv/writer/common/option_builder.h"
#include "src/tint/lang/spirv/writer/printer/printer.h"
#include "src/tint/lang/spirv/writer/raise/raise.h"
#include "src/tint/lang/wgsl/reader/lower/lower.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#endif

// Included by 'ast_printer.h', included again here for './tools/run gen' track the dependency.
#include "spirv/unified1/spirv.h"

namespace tint::spirv::writer {

Output::Output() = default;
Output::~Output() = default;
Output::Output(const Output&) = default;

Result<Output> Generate(const Program& program, const Options& options) {
    if (!program.IsValid()) {
        return Failure{program.Diagnostics()};
    }

    bool zero_initialize_workgroup_memory =
        !options.disable_workgroup_init && options.use_zero_initialize_workgroup_memory_extension;

    {
        diag::List validation_diagnostics;
        if (!ValidateBindingOptions(options, validation_diagnostics)) {
            return Failure{validation_diagnostics};
        }
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

        // Raise from core-dialect to SPIR-V-dialect.
        if (auto res = raise::Raise(ir, options); !res) {
            return std::move(res.Failure());
        }

        // Generate the SPIR-V code.
        auto spirv = Print(ir, zero_initialize_workgroup_memory);
        if (!spirv) {
            return std::move(spirv.Failure());
        }
        output.spirv = std::move(spirv.Get());
#else
        return Failure{"use_tint_ir requires building with TINT_BUILD_WGSL_READER"};
#endif
    } else {
        // Sanitize the program.
        auto sanitized_result = Sanitize(program, options);
        if (!sanitized_result.program.IsValid()) {
            return Failure{sanitized_result.program.Diagnostics()};
        }

        // Generate the SPIR-V code.
        auto impl = std::make_unique<ASTPrinter>(
            sanitized_result.program, zero_initialize_workgroup_memory,
            options.experimental_require_subgroup_uniform_control_flow);
        if (!impl->Generate()) {
            return Failure{impl->Diagnostics()};
        }
        output.spirv = std::move(impl->Result());
    }

    return output;
}

}  // namespace tint::spirv::writer
