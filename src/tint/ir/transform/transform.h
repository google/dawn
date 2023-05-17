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

#ifndef SRC_TINT_IR_TRANSFORM_TRANSFORM_H_
#define SRC_TINT_IR_TRANSFORM_TRANSFORM_H_

#include "src/tint/transform/transform.h"

#include <utility>

#include "src/tint/utils/castable.h"

// Forward declarations
namespace tint::ir {
class Module;
}  // namespace tint::ir

namespace tint::ir::transform {

/// Interface for IR Module transforms.
class Transform : public utils::Castable<Transform, tint::transform::Transform> {
  public:
    /// Constructor
    Transform();
    /// Destructor
    ~Transform() override;

    /// Run the transform on @p module
    /// @param module the source module to transform
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    virtual void Run(ir::Module* module, const DataMap& inputs, DataMap& outputs) const = 0;
};

}  // namespace tint::ir::transform

#endif  // SRC_TINT_IR_TRANSFORM_TRANSFORM_H_
