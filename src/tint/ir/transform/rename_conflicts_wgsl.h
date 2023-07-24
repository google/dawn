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

#ifndef SRC_TINT_IR_TRANSFORM_RENAME_CONFLICTS_WGSL_H_
#define SRC_TINT_IR_TRANSFORM_RENAME_CONFLICTS_WGSL_H_

#include "src/tint/ir/transform/transform.h"

// Forward declarations
namespace tint::ir {
class BuiltinCall;
}  // namespace tint::ir
namespace tint::type {
class Type;
}  // namespace tint::type

namespace tint::ir::transform {

/// RenameConflictsWGSL is a transform that renames declarations which prevent identifiers from
/// resolving to the correct declaration, and those with identical identifiers declared in the same
/// scope.
class RenameConflictsWGSL final : public utils::Castable<RenameConflictsWGSL, Transform> {
  public:
    /// Constructor
    RenameConflictsWGSL();
    /// Destructor
    ~RenameConflictsWGSL() override;

    /// @copydoc Transform::Run
    void Run(ir::Module* module, const DataMap& inputs, DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::ir::transform

#endif  // SRC_TINT_IR_TRANSFORM_RENAME_CONFLICTS_WGSL_H_
