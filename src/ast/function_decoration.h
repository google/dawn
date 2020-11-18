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

#ifndef SRC_AST_FUNCTION_DECORATION_H_
#define SRC_AST_FUNCTION_DECORATION_H_

#include <memory>
#include <ostream>
#include <vector>

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

class StageDecoration;
class WorkgroupDecoration;

/// A decoration attached to a function
class FunctionDecoration : public Decoration {
 public:
  /// The kind of decoration that this type represents
  static constexpr const DecorationKind Kind = DecorationKind::kFunction;

  ~FunctionDecoration() override;

  /// @param kind the decoration kind
  /// @return true if this Decoration is of the (or derives from) the given
  /// kind.
  bool IsKind(DecorationKind kind) const override;

  /// @returns true if this is a stage decoration
  virtual bool IsStage() const;
  /// @returns true if this is a workgroup decoration
  virtual bool IsWorkgroup() const;

  /// @returns the decoration as a stage decoration
  const StageDecoration* AsStage() const;
  /// @returns the decoration as a workgroup decoration
  const WorkgroupDecoration* AsWorkgroup() const;

 protected:
  /// Constructor
  /// @param kind the decoration kind
  /// @param source the source of this decoration
  FunctionDecoration(DecorationKind kind, const Source& source);
};

/// A list of function decorations
using FunctionDecorationList = std::vector<FunctionDecoration*>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_FUNCTION_DECORATION_H_
