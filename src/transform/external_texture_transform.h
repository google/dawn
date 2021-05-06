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

#ifndef SRC_TRANSFORM_EXTERNAL_TEXTURE_TRANSFORM_H_
#define SRC_TRANSFORM_EXTERNAL_TEXTURE_TRANSFORM_H_

#include <utility>

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// Because an external texture is comprised of 1-3 texture views we can simply
/// transform external textures into the appropriate number of sampled textures.
/// This allows us to share SPIR-V/HLSL writer paths for sampled textures
/// instead of adding dedicated writer paths for external textures.
/// ExternalTextureTransform performs this transformation.
class ExternalTextureTransform : public Transform {
 public:
  /// Constructor
  ExternalTextureTransform();
  /// Destructor
  ~ExternalTextureTransform() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_EXTERNAL_TEXTURE_TRANSFORM_H_
