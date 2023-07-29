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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_STD140_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_STD140_H_

#include "src/tint/lang/core/ir/transform/transform.h"

namespace tint::ir::transform {

/// Std140 is a transform that rewrites matrix types in the uniform address space to conform to
/// GLSL's std140 layout rules.
class Std140 final : public Castable<Std140, Transform> {
  public:
    /// Constructor
    Std140();
    /// Destructor
    ~Std140() override;

    /// @copydoc Transform::Run
    void Run(ir::Module* module) const override;
};

}  // namespace tint::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_STD140_H_
