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

#ifndef SRC_AST_RETURN_STATEMENT_H_
#define SRC_AST_RETURN_STATEMENT_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// A return statement
class ReturnStatement : public Statement {
 public:
  /// Constructor
  ReturnStatement();
  /// Constructor
  /// @param source the source information
  explicit ReturnStatement(const Source& source);
  /// Constructor
  /// @param value the return value
  explicit ReturnStatement(std::unique_ptr<Expression> value);
  /// Constructor
  /// @param source the return statement source
  /// @param value the return value
  ReturnStatement(const Source& source, std::unique_ptr<Expression> value);
  /// Move constructor
  ReturnStatement(ReturnStatement&&);
  ~ReturnStatement() override;

  /// Sets the value
  /// @param value the value
  void set_value(std::unique_ptr<Expression> value) {
    value_ = std::move(value);
  }
  /// @returns the value
  Expression* value() const { return value_.get(); }
  /// @returns true if the return has a value
  bool has_value() const { return value_ != nullptr; }

  /// @returns true if this is a return statement
  bool IsReturn() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  ReturnStatement(const ReturnStatement&) = delete;

  std::unique_ptr<Expression> value_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_RETURN_STATEMENT_H_
