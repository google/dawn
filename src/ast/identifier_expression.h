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

#ifndef SRC_AST_IDENTIFIER_EXPRESSION_H_
#define SRC_AST_IDENTIFIER_EXPRESSION_H_

#include <memory>
#include <string>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/intrinsic.h"
#include "src/symbol.h"

namespace tint {
namespace ast {

/// An identifier expression
class IdentifierExpression : public Castable<IdentifierExpression, Expression> {
 public:
  /// Constructor
  /// @param source the source
  /// @param sym the symbol for the identifier
  /// @param name the name
  IdentifierExpression(const Source& source,
                       Symbol sym,
                       const std::string& name);
  /// Move constructor
  IdentifierExpression(IdentifierExpression&&);
  ~IdentifierExpression() override;

  /// @returns the symbol for the identifier
  Symbol symbol() const { return sym_; }

  /// Sets the intrinsic for this identifier
  /// @param i the intrinsic to set
  void set_intrinsic(Intrinsic i) { intrinsic_ = i; }
  /// @returns the intrinsic this identifier represents
  Intrinsic intrinsic() const { return intrinsic_; }

  /// Sets the intrinsic signature
  /// @param s the intrinsic signature to set
  void set_intrinsic_signature(std::unique_ptr<intrinsic::Signature> s) {
    intrinsic_sig_ = std::move(s);
  }
  /// @returns the intrinsic signature for this identifier.
  const intrinsic::Signature* intrinsic_signature() const {
    return intrinsic_sig_.get();
  }

  /// @returns true if this identifier is for an intrinsic
  bool IsIntrinsic() const { return intrinsic_ != Intrinsic::kNone; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param ctx the clone context
  /// @return the newly cloned node
  IdentifierExpression* Clone(CloneContext* ctx) const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  IdentifierExpression(const IdentifierExpression&) = delete;

  Symbol const sym_;
  std::string const name_;

  Intrinsic intrinsic_ = Intrinsic::kNone;               // Semantic info
  std::unique_ptr<intrinsic::Signature> intrinsic_sig_;  // Semantic info
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_IDENTIFIER_EXPRESSION_H_
