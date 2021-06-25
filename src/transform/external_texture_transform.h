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
class ExternalTextureTransform
    : public Castable<ExternalTextureTransform, Transform> {
 public:
  /// Constructor
  ExternalTextureTransform();
  /// Destructor
  ~ExternalTextureTransform() override;

 protected:
  /// Runs the transform using the CloneContext built for transforming a
  /// program. Run() is responsible for calling Clone() on the CloneContext.
  /// @param ctx the CloneContext primed with the input program and
  /// ProgramBuilder
  /// @param inputs optional extra transform-specific input data
  /// @param outputs optional extra transform-specific output data
  void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_EXTERNAL_TEXTURE_TRANSFORM_H_
