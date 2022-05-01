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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_EXPRESSION_SIZE_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_EXPRESSION_SIZE_H_

#include <unordered_map>

#include "src/tint/ast/expression.h"
#include "src/tint/program.h"

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
