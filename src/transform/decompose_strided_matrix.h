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

#ifndef SRC_TRANSFORM_DECOMPOSE_STRIDED_MATRIX_H_
#define SRC_TRANSFORM_DECOMPOSE_STRIDED_MATRIX_H_

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// DecomposeStridedMatrix transforms replaces matrix members of storage or
/// uniform buffer structures, that have a [[stride]] decoration, into an array
/// of N column vectors.
/// This transform is used by the SPIR-V reader to handle the SPIR-V
/// MatrixStride decoration.
class DecomposeStridedMatrix
    : public Castable<DecomposeStridedMatrix, Transform> {
 public:
  /// Constructor
  DecomposeStridedMatrix();

  /// Destructor
  ~DecomposeStridedMatrix() override;

  /// @param program the program to inspect
  /// @returns true if this transform should be run for the given program
  static bool ShouldRun(const Program* program);

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

#endif  // SRC_TRANSFORM_DECOMPOSE_STRIDED_MATRIX_H_
