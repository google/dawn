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

#include "src/tint/reader/spirv/parser.h"

#include <utility>

#include "src/tint/reader/spirv/parser_impl.h"
#include "src/tint/transform/decompose_strided_array.h"
#include "src/tint/transform/decompose_strided_matrix.h"
#include "src/tint/transform/manager.h"
#include "src/tint/transform/remove_unreachable_statements.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/spirv_atomic.h"
#include "src/tint/transform/unshadow.h"

namespace tint::reader::spirv {

Program Parse(const std::vector<uint32_t>& input, const Options& options) {
    ParserImpl parser(input);
    bool parsed = parser.Parse();

    ProgramBuilder& builder = parser.builder();
    if (!parsed) {
        // TODO(bclayton): Migrate spirv::ParserImpl to using diagnostics.
        builder.Diagnostics().add_error(diag::System::Reader, parser.error());
        return Program(std::move(builder));
    }

    if (options.allow_non_uniform_derivatives) {
        // Suppress errors regarding non-uniform derivative operations if requested, by adding a
        // diagnostic directive to the module.
        builder.DiagnosticDirective(builtin::DiagnosticSeverity::kOff, "derivative_uniformity");
    }

    // The SPIR-V parser can construct disjoint AST nodes, which is invalid for
    // the Resolver. Clone the Program to clean these up.
    builder.SetResolveOnBuild(false);
    Program program_with_disjoint_ast(std::move(builder));

    ProgramBuilder output;
    CloneContext(&output, &program_with_disjoint_ast, false).Clone();
    auto program = Program(std::move(output));
    if (!program.IsValid()) {
        return program;
    }

    transform::Manager manager;
    manager.Add<transform::Unshadow>();
    manager.Add<transform::SimplifyPointers>();
    manager.Add<transform::DecomposeStridedMatrix>();
    manager.Add<transform::DecomposeStridedArray>();
    manager.Add<transform::RemoveUnreachableStatements>();
    manager.Add<transform::SpirvAtomic>();
    return manager.Run(&program).program;
}

}  // namespace tint::reader::spirv
