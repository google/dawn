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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_JUMP_TRACKER_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_JUMP_TRACKER_H_

#include <unordered_set>

#include "src/tint/lang/wgsl/ast/statement.h"
#include "src/tint/lang/wgsl/program/program.h"

namespace tint::fuzzers::ast_fuzzer {

/// This class computes information on which statements contain loop breaks and returns.
/// It could be extended to handle other jumps, such as switch breaks and loop continues, should
/// such information prove useful.
class JumpTracker {
  public:
    /// Initializes jump tracking information for the given program.
    /// @param program - the program for which jumps will be tracked;
    ///     must remain in scope as long as this instance exists.
    explicit JumpTracker(const Program& program);

    /// Indicates whether a statement contains a break statement for the innermost loop (if any).
    /// @param statement - the statement of interest.
    /// @return true if and only if the statement is, or contains, a break for the innermost
    ///     enclosing loop.
    bool ContainsBreakForInnermostLoop(const ast::Statement& statement) const {
        return contains_break_for_innermost_loop_.count(&statement) > 0;
    }

    /// Indicates whether a statement contains a return statement.
    /// @param statement - the statement of interest.
    /// @return true if and only if the statement is, or contains, a return statement.
    bool ContainsReturn(const ast::Statement& statement) const {
        return contains_return_.count(&statement) > 0;
    }

  private:
    std::unordered_set<const ast::Statement*> contains_break_for_innermost_loop_;
    std::unordered_set<const ast::Statement*> contains_return_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_JUMP_TRACKER_H_
