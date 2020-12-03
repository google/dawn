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

#ifndef SRC_TRANSFORM_EMIT_VERTEX_POINT_SIZE_TRANSFORM_H_
#define SRC_TRANSFORM_EMIT_VERTEX_POINT_SIZE_TRANSFORM_H_

#include "src/transform/transformer.h"

namespace tint {
namespace transform {

/// EmitVertexPointSizeTransform is a Transformer that adds a PointSize builtin
/// global output variable to the module which is assigned 1.0 as the new first
/// statement for all vertex stage entry points.
/// If the module does not contain a vertex pipeline stage entry point then then
/// this transformer is a no-op.
class EmitVertexPointSizeTransform : public Transformer {
 public:
  /// Constructor
  /// @param mod the module transform
  explicit EmitVertexPointSizeTransform(ast::Module* mod);
  ~EmitVertexPointSizeTransform() override;

  /// Users of Tint should register the transform with transform manager and
  /// invoke its Run(), instead of directly calling the transform's Run().
  /// Calling Run() directly does not perform module state cleanup operations.
  /// @returns true if the transformation was successful
  bool Run() override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_EMIT_VERTEX_POINT_SIZE_TRANSFORM_H_
