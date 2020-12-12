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

#ifndef SRC_AST_LITERAL_H_
#define SRC_AST_LITERAL_H_

#include <string>

#include "src/ast/node.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {

/// Base class for a literal value
class Literal : public Castable<Literal, Node> {
 public:
  ~Literal() override;

  /// @returns the type of the literal
  type::Type* type() const { return type_; }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

  /// @returns the literal as a string
  virtual std::string to_str() const = 0;

  /// @returns the name for this literal. This name is unique to this value.
  virtual std::string name() const = 0;

 protected:
  /// Constructor
  /// @param source the input source
  /// @param type the type of the literal
  explicit Literal(const Source& source, type::Type* type);

 private:
  type::Type* type_ = nullptr;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_LITERAL_H_
