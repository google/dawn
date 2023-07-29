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

#include "src/tint/lang/spirv/reader/reader.h"

#include <utility>

#include "src/tint/lang/spirv/reader/ast_parser/ast_parser.h"
#include "src/tint/lang/wgsl/ast/transform/decompose_strided_array.h"
#include "src/tint/lang/wgsl/ast/transform/decompose_strided_matrix.h"
#include "src/tint/lang/wgsl/ast/transform/fold_trivial_lets.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/remove_unreachable_statements.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/transform/spirv_atomic.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::spirv::reader {

Program Parse(const std::vector<uint32_t>& input, const Options& options) {
    ASTParser parser(input);
    bool parsed = parser.Parse();

    ProgramBuilder& builder = parser.builder();
    if (!parsed) {
        // TODO(bclayton): Migrate ASTParser to using diagnostics.
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
    Program program_with_disjoint_ast(std::move(builder));

    ProgramBuilder output;
    program::CloneContext(&output, &program_with_disjoint_ast, false).Clone();
    auto program = Program(resolver::Resolve(output));
    if (!program.IsValid()) {
        return program;
    }

    ast::transform::Manager manager;
    ast::transform::DataMap outputs;
    manager.Add<ast::transform::Unshadow>();
    manager.Add<ast::transform::SimplifyPointers>();
    manager.Add<ast::transform::FoldTrivialLets>();
    manager.Add<ast::transform::DecomposeStridedMatrix>();
    manager.Add<ast::transform::DecomposeStridedArray>();
    manager.Add<ast::transform::RemoveUnreachableStatements>();
    manager.Add<ast::transform::SpirvAtomic>();
    return manager.Run(&program, {}, outputs);
}

}  // namespace tint::spirv::reader
