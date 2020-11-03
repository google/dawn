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

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

Decoration::~Decoration() = default;

std::ostream& operator<<(std::ostream& out, DecorationKind data) {
  switch (data) {
    case DecorationKind::kArray:
      return out << "array";
    case DecorationKind::kFunction:
      return out << "function";
    case DecorationKind::kStruct:
      return out << "struct";
    case DecorationKind::kStructMember:
      return out << "struct member";
    case DecorationKind::kVariable:
      return out << "variable";
  }
  return out << "<unknown>";
}

}  // namespace ast
}  // namespace tint
