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

#include "src/ast/intrinsic.h"

namespace tint {
namespace ast {
namespace intrinsic {

bool IsDerivative(const std::string& name) {
  return name == "dpdx" || name == "dpdx_fine" || name == "dpdx_coarse" ||
         name == "dpdy" || name == "dpdy_fine" || name == "dpdy_coarse" ||
         name == "fwidth" || name == "fwidth_fine" || name == "fwidth_coarse";
}

bool IsFloatClassificationIntrinsic(const std::string& name) {
  return name == "is_finite" || name == "is_inf" || name == "is_nan" ||
         name == "is_normal";
}

bool IsIntrinsic(const std::string& name) {
  return IsDerivative(name) || name == "all" || name == "any" ||
         IsFloatClassificationIntrinsic(name) || name == "dot" ||
         name == "outer_product";
}

}  // namespace intrinsic
}  // namespace ast
}  // namespace tint
