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

#ifndef SRC_TRANSFORM_PAD_ARRAY_ELEMENTS_H_
#define SRC_TRANSFORM_PAD_ARRAY_ELEMENTS_H_

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// PadArrayElements is a transform that replaces array types with an explicit
/// stride that is larger than the implicit stride, with an array of a new
/// structure type. This structure holds with a single field of the element
/// type, decorated with a [[size]] decoration to pad the structure to the
/// required array stride. The new array types have no explicit stride,
/// structure size is equal to the desired stride.
/// Array index expressions and constructors are also adjusted to deal with this
/// structure element type.
/// This transform helps with backends that cannot directly return arrays or use
/// them as parameters.
class PadArrayElements : public Castable<PadArrayElements, Transform> {
 public:
  /// Constructor
  PadArrayElements();

  /// Destructor
  ~PadArrayElements() override;

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

#endif  // SRC_TRANSFORM_PAD_ARRAY_ELEMENTS_H_
