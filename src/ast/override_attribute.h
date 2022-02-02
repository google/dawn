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

#ifndef SRC_AST_OVERRIDE_ATTRIBUTE_H_
#define SRC_AST_OVERRIDE_ATTRIBUTE_H_

#include <string>

#include "src/ast/attribute.h"

namespace tint {
namespace ast {

/// An override attribute
class OverrideAttribute : public Castable<OverrideAttribute, Attribute> {
 public:
  /// Create an override attribute with no specified id.
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node
  OverrideAttribute(ProgramID pid, const Source& src);
  /// Create an override attribute with a specific id value.
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node
  /// @param val the override value
  OverrideAttribute(ProgramID pid, const Source& src, uint32_t val);
  ~OverrideAttribute() override;

  /// @returns the WGSL name for the attribute
  std::string Name() const override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  const OverrideAttribute* Clone(CloneContext* ctx) const override;

  /// True if an override id was specified
  const bool has_value;

  /// The override id value
  const uint32_t value;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_OVERRIDE_ATTRIBUTE_H_
