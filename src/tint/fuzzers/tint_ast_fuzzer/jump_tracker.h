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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_JUMP_TRACKER_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_JUMP_TRACKER_H_

#include <unordered_set>

#include "src/tint/ast/statement.h"
#include "src/tint/program.h"

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
