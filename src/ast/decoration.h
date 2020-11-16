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
  kFunction,
  kStruct,
  kStructMember,
  kVariable
};

std::ostream& operator<<(std::ostream& out, DecorationKind data);

/// The base class for all decorations
class Decoration : public Node {
 public:
  ~Decoration() override;

  /// @return the decoration kind
  DecorationKind GetKind() const { return kind_; }

  /// @return true if this decoration is of (or derives from) type |TO|
  template <typename TO>
  bool Is() const {
    return GetKind() == TO::Kind;
  }

  /// @returns true if the node is valid
  bool IsValid() const override;

 protected:
  /// Constructor
  /// @param kind represents the derived type
  /// @param source the source of this decoration
  Decoration(DecorationKind kind, const Source& source)
      : Node(source), kind_(kind) {}

 private:
  DecorationKind const kind_;
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
