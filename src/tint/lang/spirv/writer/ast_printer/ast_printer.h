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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_AST_PRINTER_H_
#define SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_AST_PRINTER_H_

#include <string>
#include <vector>

#include "src/tint/lang/spirv/writer/ast_printer/builder.h"
#include "src/tint/lang/spirv/writer/common/binary_writer.h"
#include "src/tint/lang/spirv/writer/common/options.h"
#include "src/tint/lang/wgsl/program/program.h"

namespace tint::spirv::writer {

/// The result of sanitizing a program for generation.
struct SanitizedResult {
    /// The sanitized program.
    Program program;
};

/// Sanitize a program in preparation for generating SPIR-V.
/// @program The program to sanitize
/// @param options The SPIR-V generator options.
SanitizedResult Sanitize(const Program& program, const Options& options);

/// Implementation class for SPIR-V generator
class ASTPrinter {
  public:
    /// Constructor
    /// @param program the program to generate
    /// @param zero_initialize_workgroup_memory `true` to initialize all the
    /// variables in the Workgroup address space with OpConstantNull
    /// @param experimental_require_subgroup_uniform_control_flow `true` to require
    /// `SPV_KHR_subgroup_uniform_control_flow` extension and `SubgroupUniformControlFlowKHR`
    /// execution mode for compute stage entry points.
    ASTPrinter(const Program& program,
               bool zero_initialize_workgroup_memory,
               bool experimental_require_subgroup_uniform_control_flow);

    /// @returns true on successful generation; false otherwise
    bool Generate();

    /// @returns the result data
    const std::vector<uint32_t>& Result() const { return writer_.Result(); }

    /// @returns the list of diagnostics raised by the generator
    diag::List Diagnostics() const { return builder_.Diagnostics(); }

  private:
    Builder builder_;
    BinaryWriter writer_;
};

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_AST_PRINTER_H_
