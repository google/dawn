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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_EXPRESSION_SIZE_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_EXPRESSION_SIZE_H_

#include <unordered_map>

#include "src/tint/lang/wgsl/ast/expression.h"
#include "src/tint/lang/wgsl/program/program.h"

namespace tint::fuzzers::ast_fuzzer {

/// This class computes the size of the subtree rooted at each expression in a
/// program, and allows these sizes to be subsequently queried.
class ExpressionSize {
  public:
    /// Initializes expression size information for the given program.
    /// @param program - the program for which expression sizes will be computed;
    ///     must remain in scope as long as this instance exists.
    explicit ExpressionSize(const Program& program);

    /// Returns the size of the subtree rooted at the given expression.
    /// @param expression - the expression whose size should be returned.
    /// @return the size of the subtree rooted at `expression`.
    size_t operator()(const ast::Expression* expression) const {
        return expr_to_size_.at(expression);
    }

  private:
    std::unordered_map<const ast::Expression*, size_t> expr_to_size_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_EXPRESSION_SIZE_H_
