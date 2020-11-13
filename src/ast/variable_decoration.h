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

#ifndef SRC_AST_VARIABLE_DECORATION_H_
#define SRC_AST_VARIABLE_DECORATION_H_

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

class BindingDecoration;
class BuiltinDecoration;
class ConstantIdDecoration;
class LocationDecoration;
class SetDecoration;

/// A decoration attached to a variable
class VariableDecoration : public Decoration {
 public:
  /// The kind of decoration that this type represents
  static constexpr DecorationKind Kind = DecorationKind::kVariable;

  ~VariableDecoration() override;

  /// @returns true if this is a binding decoration
  virtual bool IsBinding() const;
  /// @returns true if this is a builtin decoration
  virtual bool IsBuiltin() const;
  /// @returns true if this is a constant id decoration
  virtual bool IsConstantId() const;
  /// @returns true if this is a location decoration
  virtual bool IsLocation() const;
  /// @returns true if this is a set decoration
  virtual bool IsSet() const;

  /// @returns the decoration as a binding decoration
  BindingDecoration* AsBinding();
  /// @returns the decoration as a builtin decoration
  BuiltinDecoration* AsBuiltin();
  /// @returns the decoration as a constant id decoration
  ConstantIdDecoration* AsConstantId();
  /// @returns the decoration as a location decoration
  LocationDecoration* AsLocation();
  /// @returns the decoration as a set decoration
  SetDecoration* AsSet();

 protected:
  /// Constructor
  /// @param source the source of this decoration
  explicit VariableDecoration(const Source& source);
};

/// A list of unique variable decorations
using VariableDecorationList = std::vector<std::unique_ptr<VariableDecoration>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_VARIABLE_DECORATION_H_
