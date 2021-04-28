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

namespace tint {
namespace ast {

/// Base class for a literal value
class Literal : public Castable<Literal, Node> {
 public:
  ~Literal() override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

  /// @param sem the semantic info for the program
  /// @returns the literal as a string
  virtual std::string to_str(const sem::Info& sem) const = 0;

  /// @returns the name for this literal. This name is unique to this value.
  virtual std::string name() const = 0;

 protected:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the input source
  Literal(ProgramID program_id, const Source& source);
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_LITERAL_H_
