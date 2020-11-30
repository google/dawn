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
class Decoration : public Node {
 public:
  ~Decoration() override;

  /// @return the decoration kind
  virtual DecorationKind GetKind() const = 0;

  /// @param kind the decoration kind
  /// @return true if this Decoration is of the (or derives from) the given
  /// kind.
  virtual bool IsKind(DecorationKind kind) const = 0;

  /// @return true if this decoration is of (or derives from) type |TO|
  template <typename TO>
  bool Is() const {
    return IsKind(TO::Kind);
  }

  /// @returns true if the node is valid
  bool IsValid() const override;

 protected:
  /// Constructor
  /// @param source the source of this decoration
  explicit Decoration(const Source& source) : Node(source) {}
};

/// As dynamically casts |deco| to the target type |TO|.
/// @return the cast decoration, or nullptr if |deco| is not of the type |TO|.
template <typename TO>
TO* As(Decoration* deco) {
  if (deco == nullptr) {
    return nullptr;
  }
  if (deco->Is<TO>()) {
    return static_cast<TO*>(deco);
  }
  return nullptr;
}

/// A list of decorations
using DecorationList = std::vector<Decoration*>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_DECORATION_H_
