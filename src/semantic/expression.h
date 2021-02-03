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

#ifndef SRC_SEMANTIC_EXPRESSION_H_
#define SRC_SEMANTIC_EXPRESSION_H_

#include "src/semantic/node.h"

namespace tint {

// Forward declarations
namespace type {

class Type;

}  // namespace type

namespace semantic {

/// Expression holds the semantic information for expression nodes.
class Expression : public Castable<Expression, Node> {
 public:
  /// Constructor
  /// @param type the resolved type of the expression
  explicit Expression(type::Type* type);

  /// @return the resolved type of the expression
  type::Type* Type() const { return type_; }

 private:
  type::Type* const type_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_EXPRESSION_H_
