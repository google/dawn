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
  return name == "dpdxCoarse" || name == "dpdyCoarse" || name == "fwidthCoarse";
}

bool IsFineDerivative(const std::string& name) {
  return name == "dpdxFine" || name == "dpdyFine" || name == "fwidthFine";
}

bool IsDerivative(const std::string& name) {
  return name == "dpdx" || name == "dpdy" || name == "fwidth" ||
         IsCoarseDerivative(name) || IsFineDerivative(name);
}

bool IsFloatClassificationIntrinsic(const std::string& name) {
  return name == "isFinite" || name == "isInf" || name == "isNan" ||
         name == "isNormal";
}

bool IsTextureOperationIntrinsic(const std::string& name) {
  return name == "textureLoad" || name == "textureSample" ||
         name == "textureSampleLevel" || name == "textureSampleBias" ||
         name == "textureSampleCompare";
}

bool IsIntrinsic(const std::string& name) {
  return IsDerivative(name) || name == "all" || name == "any" ||
         IsFloatClassificationIntrinsic(name) ||
         IsTextureOperationIntrinsic(name) || name == "dot" ||
         name == "outerProduct" || name == "select";
}

}  // namespace intrinsic
}  // namespace ast
}  // namespace tint
