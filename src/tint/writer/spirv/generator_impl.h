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

#ifndef SRC_TINT_WRITER_SPIRV_GENERATOR_IMPL_H_
#define SRC_TINT_WRITER_SPIRV_GENERATOR_IMPL_H_

#include <string>
#include <vector>

#include "src/tint/program.h"
#include "src/tint/writer/spirv/binary_writer.h"
#include "src/tint/writer/spirv/builder.h"
#include "src/tint/writer/spirv/generator.h"

namespace tint::writer::spirv {

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
class GeneratorImpl {
  public:
    /// Constructor
    /// @param program the program to generate
    /// @param zero_initialize_workgroup_memory `true` to initialize all the
    /// variables in the Workgroup address space with OpConstantNull
    GeneratorImpl(const Program* program, bool zero_initialize_workgroup_memory);

    /// @returns true on successful generation; false otherwise
    bool Generate();

    /// @returns the result data
    const std::vector<uint32_t>& Result() const { return writer_.result(); }

    /// @returns the list of diagnostics raised by the generator
    diag::List Diagnostics() const { return builder_.Diagnostics(); }

  private:
    Builder builder_;
    BinaryWriter writer_;
};

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_GENERATOR_IMPL_H_
