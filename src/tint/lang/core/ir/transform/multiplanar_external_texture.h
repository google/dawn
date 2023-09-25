// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_MULTIPLANAR_EXTERNAL_TEXTURE_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_MULTIPLANAR_EXTERNAL_TEXTURE_H_

#include <string>

#include "src/tint/api/options/external_texture.h"
#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// MultiplanarExternalTexture is a transform that splits texture_external bindings into two
/// separate texture_2d<f32> bindings for two possible planes, along with a uniform buffer of
/// parameters that describe how the texture should be sampled.
/// @param module the module to transform
/// @param options the external texture options
/// @returns success or failure
Result<SuccessType> MultiplanarExternalTexture(Module& module,
                                               const ExternalTextureOptions& options);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_MULTIPLANAR_EXTERNAL_TEXTURE_H_
