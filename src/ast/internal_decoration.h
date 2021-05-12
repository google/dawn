// Copyright 2021 The Tint Authors.
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

#ifndef SRC_AST_INTERNAL_DECORATION_H_
#define SRC_AST_INTERNAL_DECORATION_H_

#include <string>

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

/// A decoration used to indicate that a function is tint-internal.
/// These decorations are not produced by generators, but instead are usually
/// created by transforms for consumption by a particular backend.
class InternalDecoration : public Castable<InternalDecoration, Decoration> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  explicit InternalDecoration(ProgramID program_id);

  /// Destructor
  ~InternalDecoration() override;

  /// @return a short description of the internal decoration which will be
  /// displayed in WGSL as `[[internal(<name>)]]` (but is not parsable).
  virtual std::string Name() const = 0;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_INTERNAL_DECORATION_H_
