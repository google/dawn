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

#include "src/tint/lang/spirv/reader/ast_parser/parse.h"

#include <utility>

#include "src/tint/lang/spirv/reader/ast_lower/atomics.h"
#include "src/tint/lang/spirv/reader/ast_lower/decompose_strided_array.h"
#include "src/tint/lang/spirv/reader/ast_lower/decompose_strided_matrix.h"
#include "src/tint/lang/spirv/reader/ast_lower/fold_trivial_lets.h"
#include "src/tint/lang/spirv/reader/ast_parser/ast_parser.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/remove_unreachable_statements.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::spirv::reader::ast_parser {

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
        builder.DiagnosticDirective(wgsl::DiagnosticSeverity::kOff, "derivative_uniformity");
    }

    if (!options.allow_chromium_extensions) {
        // Check if any Chromium extensions were used.
        for (auto* enable : builder.AST().Enables()) {
            for (auto* extension : enable->extensions) {
                switch (extension->name) {
                    case wgsl::Extension::kUndefined:
                    case wgsl::Extension::kChromiumDisableUniformityAnalysis:
                    case wgsl::Extension::kChromiumExperimentalDp4A:
                    case wgsl::Extension::kChromiumExperimentalFullPtrParameters:
                    case wgsl::Extension::kChromiumExperimentalPixelLocal:
                    case wgsl::Extension::kChromiumExperimentalPushConstant:
                    case wgsl::Extension::kChromiumExperimentalReadWriteStorageTexture:
                    case wgsl::Extension::kChromiumExperimentalSubgroups:
                    case wgsl::Extension::kChromiumInternalDualSourceBlending:
                    case wgsl::Extension::kChromiumInternalRelaxedUniformLayout: {
                        StringStream ss;
                        ss << "module requires " << ToString(extension->name)
                           << ", but 'allow-chromium-extensions' was not passed";
                        builder.Diagnostics().add_error(diag::System::Reader, ss.str());
                        return Program(std::move(builder));
                    }
                    case wgsl::Extension::kF16:
                        break;
                }
            }
        }
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
    manager.Add<FoldTrivialLets>();
    manager.Add<DecomposeStridedMatrix>();
    manager.Add<DecomposeStridedArray>();
    manager.Add<ast::transform::RemoveUnreachableStatements>();
    manager.Add<Atomics>();
    return manager.Run(program, {}, outputs);
}

}  // namespace tint::spirv::reader::ast_parser
