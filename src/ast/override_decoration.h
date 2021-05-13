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

#ifndef SRC_AST_OVERRIDE_DECORATION_H_
#define SRC_AST_OVERRIDE_DECORATION_H_

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

/// An override decoration
class OverrideDecoration : public Castable<OverrideDecoration, Decoration> {
 public:
  /// Create an override decoration with no specified id.
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this decoration
  OverrideDecoration(ProgramID program_id, const Source& source);
  /// Create an override decoration with a specific id value.
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this decoration
  /// @param val the override value
  OverrideDecoration(ProgramID program_id, const Source& source, uint32_t val);
  ~OverrideDecoration() override;

  /// @returns true if an override id was specified
  uint32_t HasValue() const { return has_value_; }

  /// @returns the override id value
  uint32_t value() const { return value_; }

  /// Outputs the decoration to the given stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  OverrideDecoration* Clone(CloneContext* ctx) const override;

 private:
  bool has_value_;
  uint32_t const value_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_OVERRIDE_DECORATION_H_
