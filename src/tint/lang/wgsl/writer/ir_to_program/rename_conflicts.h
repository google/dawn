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

#ifndef SRC_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM_RENAME_CONFLICTS_H_
#define SRC_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM_RENAME_CONFLICTS_H_

#include "src/tint/lang/core/ir/transform/transform.h"

namespace tint::wgsl::writer {

/// RenameConflicts is a transform that renames declarations which prevent identifiers from
/// resolving to the correct declaration, and those with identical identifiers declared in the same
/// scope.
class RenameConflicts final : public Castable<RenameConflicts, ir::transform::Transform> {
  public:
    /// Constructor
    RenameConflicts();
    /// Destructor
    ~RenameConflicts() override;

    /// @copydoc ir::transform::Transform::Run
    void Run(ir::Module* module) const override;

  private:
    struct State;
};

}  // namespace tint::wgsl::writer

#endif  // SRC_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM_RENAME_CONFLICTS_H_
