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
SanitizedResult Sanitize(const Program* program, const Options& options);

/// Implementation class for SPIR-V generator
class ASTPrinter {
  public:
    /// Constructor
    /// @param program the program to generate
    /// @param zero_initialize_workgroup_memory `true` to initialize all the
    /// variables in the Workgroup address space with OpConstantNull
    ASTPrinter(const Program* program, bool zero_initialize_workgroup_memory);

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
