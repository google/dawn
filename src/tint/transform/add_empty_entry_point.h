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

#ifndef SRC_TINT_TRANSFORM_ADD_EMPTY_ENTRY_POINT_H_
#define SRC_TINT_TRANSFORM_ADD_EMPTY_ENTRY_POINT_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Add an empty entry point to the module, if no other entry points exist.
class AddEmptyEntryPoint final : public utils::Castable<AddEmptyEntryPoint, Transform> {
  public:
    /// Constructor
    AddEmptyEntryPoint();
    /// Destructor
    ~AddEmptyEntryPoint() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_ADD_EMPTY_ENTRY_POINT_H_
