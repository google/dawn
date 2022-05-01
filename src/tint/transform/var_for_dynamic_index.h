// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_TRANSFORM_VAR_FOR_DYNAMIC_INDEX_H_
#define SRC_TINT_TRANSFORM_VAR_FOR_DYNAMIC_INDEX_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// A transform that extracts array and matrix values that are dynamically
/// indexed to a temporary `var` local before performing the index. This
/// transform is used by the SPIR-V writer as there is no SPIR-V instruction
/// that can dynamically index a non-pointer composite.
class VarForDynamicIndex : public Transform {
  public:
    /// Constructor
    VarForDynamicIndex();

    /// Destructor
    ~VarForDynamicIndex() override;

  protected:
    /// Runs the transform using the CloneContext built for transforming a
    /// program. Run() is responsible for calling Clone() on the CloneContext.
    /// @param ctx the CloneContext primed with the input program and
    /// ProgramBuilder
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_VAR_FOR_DYNAMIC_INDEX_H_
