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

#ifndef SRC_AST_DECORATION_H_
#define SRC_AST_DECORATION_H_

#include <memory>
#include <ostream>
#include <vector>

#include "src/ast/node.h"
#include "src/source.h"

namespace tint {
namespace ast {

/// The decoration kind enumerator
enum class DecorationKind {
  kArray,
  /*|*/ kStride,
  kFunction,
  /*|*/ kStage,
  /*|*/ kWorkgroup,
  kStruct,
  kStructMember,
  /*|*/ kStructMemberOffset,
  kType,
  /*|*/ kAccess,
  kVariable,
  /*|*/ kBinding,
  /*|*/ kBuiltin,
  /*|*/ kConstantId,
  /*|*/ kLocation,
};

std::ostream& operator<<(std::ostream& out, DecorationKind data);

/// The base class for all decorations
class Decoration : public Castable<Decoration, Node> {
 public:
  ~Decoration() override;

  /// @return the decoration kind
  virtual DecorationKind GetKind() const = 0;

  /// @returns true if the node is valid
  bool IsValid() const override;

 protected:
  /// Constructor
  /// @param source the source of this decoration
  explicit Decoration(const Source& source) : Base(source) {}
};

/// A list of decorations
using DecorationList = std::vector<Decoration*>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_DECORATION_H_
