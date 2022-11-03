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

#ifndef SRC_TINT_TRANSFORM_ROBUSTNESS_H_
#define SRC_TINT_TRANSFORM_ROBUSTNESS_H_

#include <unordered_set>

#include "src/tint/transform/transform.h"

// Forward declarations
namespace tint::ast {
class IndexAccessorExpression;
class CallExpression;
}  // namespace tint::ast

namespace tint::transform {

/// This transform is responsible for clamping all array accesses to be within
/// the bounds of the array. Any access before the start of the array will clamp
/// to zero and any access past the end of the array will clamp to
/// (array length - 1).
class Robustness final : public Castable<Robustness, Transform> {
  public:
    /// Address space to be skipped in the transform
    enum class AddressSpace {
        kUniform,
        kStorage,
    };

    /// Configuration options for the transform
    struct Config final : public Castable<Config, Data> {
        /// Constructor
        Config();

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// Assignment operator
        /// @returns this Config
        Config& operator=(const Config&);

        /// Address spaces to omit from apply the transform to.
        /// This allows for optimizing on hardware that provide safe accesses.
        std::unordered_set<AddressSpace> omitted_address_spaces;
    };

    /// Constructor
    Robustness();
    /// Destructor
    ~Robustness() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_ROBUSTNESS_H_
