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

#include "src/ast/unary_method.h"

namespace tint {
namespace ast {

std::ostream& operator<<(std::ostream& out, UnaryMethod mod) {
  switch (mod) {
    case UnaryMethod::kAny: {
      out << "any";
      break;
    }
    case UnaryMethod::kAll: {
      out << "all";
      break;
    }
    case UnaryMethod::kIsNan: {
      out << "is_nan";
      break;
    }
    case UnaryMethod::kIsInf: {
      out << "is_inf";
      break;
    }
    case UnaryMethod::kIsFinite: {
      out << "is_finite";
      break;
    }
    case UnaryMethod::kIsNormal: {
      out << "is_normal";
      break;
    }
    case UnaryMethod::kDot: {
      out << "dot";
      break;
    }
    case UnaryMethod::kOuterProduct: {
      out << "outer_product";
      break;
    }
  }
  return out;
}

}  // namespace ast
}  // namespace tint
