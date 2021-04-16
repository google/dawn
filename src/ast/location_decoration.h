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

#ifndef SRC_AST_LOCATION_DECORATION_H_
#define SRC_AST_LOCATION_DECORATION_H_

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

/// A location decoration
class LocationDecoration : public Castable<LocationDecoration, Decoration> {
 public:
  /// constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this decoration
  /// @param value the location value
  LocationDecoration(ProgramID program_id,
                     const Source& source,
                     uint32_t value);
  ~LocationDecoration() override;

  /// @returns the location value
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
  LocationDecoration* Clone(CloneContext* ctx) const override;

 private:
  uint32_t const value_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_LOCATION_DECORATION_H_
