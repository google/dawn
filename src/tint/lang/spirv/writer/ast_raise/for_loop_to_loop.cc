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

#include "src/tint/lang/spirv/writer/ast_raise/for_loop_to_loop.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::writer::ForLoopToLoop);

namespace tint::spirv::writer {
namespace {

bool ShouldRun(const Program& program) {
    for (auto* node : program.ASTNodes().Objects()) {
        if (node->Is<ast::ForLoopStatement>()) {
            return true;
        }
    }
    return false;
}

}  // namespace

ForLoopToLoop::ForLoopToLoop() = default;

ForLoopToLoop::~ForLoopToLoop() = default;

ast::transform::Transform::ApplyResult ForLoopToLoop::Apply(const Program& src,
                                                            const ast::transform::DataMap&,
                                                            ast::transform::DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    ctx.ReplaceAll([&](const ast::ForLoopStatement* for_loop) -> const ast::Statement* {
        tint::Vector<const ast::Statement*, 8> stmts;
        if (auto* cond = for_loop->condition) {
            // !condition
            auto* not_cond = b.Not(ctx.Clone(cond));

            // { break; }
            auto* break_body = b.Block(b.Break());

            // if (!condition) { break; }
            stmts.Push(b.If(not_cond, break_body));
        }
        for (auto* stmt : for_loop->body->statements) {
            stmts.Push(ctx.Clone(stmt));
        }

        const ast::BlockStatement* continuing = nullptr;
        if (auto* cont = for_loop->continuing) {
            continuing = b.Block(ctx.Clone(cont));
        }

        auto* body = b.Block(stmts);
        auto* loop = b.Loop(body, continuing);

        if (auto* init = for_loop->initializer) {
            return b.Block(ctx.Clone(init), loop);
        }

        return loop;
    });

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::spirv::writer
