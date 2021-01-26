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

#ifndef SRC_TRANSFORM_EMIT_VERTEX_POINT_SIZE_H_
#define SRC_TRANSFORM_EMIT_VERTEX_POINT_SIZE_H_

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// EmitVertexPointSize is a Transform that adds a PointSize builtin global
/// output variable to the program which is assigned 1.0 as the new first
/// statement for all vertex stage entry points.
/// If the program does not contain a vertex pipeline stage entry point then
/// then this transform is a no-op.
class EmitVertexPointSize : public Transform {
 public:
  /// Constructor
  EmitVertexPointSize();
  /// Destructor
  ~EmitVertexPointSize() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @note Users of Tint should register the transform with transform manager
  /// and invoke its Run(), instead of directly calling the transform's Run().
  /// Calling Run() directly does not perform program state cleanup operations.
  /// @param program the source program to transform
  /// @returns the transformation result
  Output Run(const Program* program) override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_EMIT_VERTEX_POINT_SIZE_H_
