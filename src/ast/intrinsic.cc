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

bool IsCoarseDerivative(const std::string& name) {
  return name == "dpdx_coarse" || name == "dpdy_coarse" ||
         name == "fwidth_coarse";
}

bool IsFineDerivative(const std::string& name) {
  return name == "dpdx_fine" || name == "dpdy_fine" || name == "fwidth_fine";
}

bool IsDerivative(const std::string& name) {
  return name == "dpdx" || name == "dpdy" || name == "fwidth" ||
         IsCoarseDerivative(name) || IsFineDerivative(name);
}

bool IsFloatClassificationIntrinsic(const std::string& name) {
  return name == "is_finite" || name == "is_inf" || name == "is_nan" ||
         name == "is_normal";
}

bool IsTextureOperationIntrinsic(const std::string& name) {
  return name == "texture_load" || name == "texture_sample" ||
         name == "texture_sample_level" || name == "texture_sample_bias" ||
         name == "texture_sample_compare";
}

bool IsIntrinsic(const std::string& name) {
  return IsDerivative(name) || name == "all" || name == "any" ||
         IsFloatClassificationIntrinsic(name) ||
         IsTextureOperationIntrinsic(name) || name == "dot" ||
         name == "outer_product" || name == "select";
}

}  // namespace intrinsic
}  // namespace ast
}  // namespace tint
