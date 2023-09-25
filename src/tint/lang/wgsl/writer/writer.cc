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

#include "src/tint/lang/wgsl/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"

#if TINT_BUILD_SYNTAX_TREE_WRITER
#include "src/tint/lang/wgsl/writer/syntax_tree_printer/syntax_tree_printer.h"
#endif  // TINT_BUILD_SYNTAX_TREE_WRITER

namespace tint::wgsl::writer {

Result<Output> Generate(const Program& program, const Options& options) {
    (void)options;

    Output output;
#if TINT_BUILD_SYNTAX_TREE_WRITER
    if (options.use_syntax_tree_writer) {
        // Generate the WGSL code.
        auto impl = std::make_unique<SyntaxTreePrinter>(program);
        if (!impl->Generate()) {
            return Failure{impl->Diagnostics()};
        }
        output.wgsl = impl->Result();
    } else  // NOLINT(readability/braces)
#endif
    {
        // Generate the WGSL code.
        auto impl = std::make_unique<ASTPrinter>(program);
        if (!impl->Generate()) {
            return Failure{impl->Diagnostics()};
        }
        output.wgsl = impl->Result();
    }

    return output;
}

Result<Output> WgslFromIR(core::ir::Module& module) {
    // core-dialect -> WGSL-dialect
    if (auto res = raise::Raise(module); !res) {
        return res.Failure();
    }

    auto program = IRToProgram(module);
    if (!program.IsValid()) {
        return Failure{program.Diagnostics()};
    }

    return Generate(program, Options{});
}

}  // namespace tint::wgsl::writer
