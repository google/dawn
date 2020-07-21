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

#ifndef SRC_AST_CALL_STATEMENT_H_
#define SRC_AST_CALL_STATEMENT_H_

#include <memory>
#include <utility>

#include "src/ast/call_expression.h"
#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// A call expression
class CallStatement : public Statement {
 public:
  /// Constructor
  CallStatement();
  /// Constructor
  /// @param call the function
  explicit CallStatement(std::unique_ptr<CallExpression> call);
  /// Move constructor
  CallStatement(CallStatement&&);
  ~CallStatement() override;

  /// Sets the call expression
  /// @param call the call
  void set_expr(std::unique_ptr<CallExpression> call) {
    call_ = std::move(call);
  }
  /// @returns the call expression
  CallExpression* expr() const { return call_.get(); }

  /// @returns true if this is a call statement
  bool IsCall() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  CallStatement(const CallStatement&) = delete;

  std::unique_ptr<CallExpression> call_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CALL_STATEMENT_H_
