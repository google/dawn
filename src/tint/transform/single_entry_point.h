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

#ifndef SRC_TINT_TRANSFORM_SINGLE_ENTRY_POINT_H_
#define SRC_TINT_TRANSFORM_SINGLE_ENTRY_POINT_H_

#include <string>

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Strip all but one entry point a module.
///
/// All module-scope variables, types, and functions that are not used by the
/// target entry point will also be removed.
class SingleEntryPoint final : public utils::Castable<SingleEntryPoint, Transform> {
  public:
    /// Configuration options for the transform
    struct Config final : public utils::Castable<Config, Data> {
        /// Constructor
        /// @param entry_point the name of the entry point to keep
        explicit Config(std::string entry_point = "");

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// Assignment operator
        /// @returns this Config
        Config& operator=(const Config&);

        /// The name of the entry point to keep.
        std::string entry_point_name;
    };

    /// Constructor
    SingleEntryPoint();

    /// Destructor
    ~SingleEntryPoint() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_SINGLE_ENTRY_POINT_H_
