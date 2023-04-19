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

#ifndef SRC_TINT_TRANSFORM_MANAGER_H_
#define SRC_TINT_TRANSFORM_MANAGER_H_

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// A collection of Transforms that act as a single Transform.
/// The inner transforms will execute in the appended order.
/// If any inner transform fails the manager will return immediately and
/// the error can be retrieved with the Output's diagnostics.
class Manager final : public utils::Castable<Manager, Transform> {
  public:
    /// Constructor
    Manager();
    ~Manager() override;

    /// Add pass to the manager
    /// @param transform the transform to append
    void append(std::unique_ptr<Transform> transform) {
        transforms_.push_back(std::move(transform));
    }

    /// Add pass to the manager of type `T`, constructed with the provided
    /// arguments.
    /// @param args the arguments to forward to the `T` initializer
    template <typename T, typename... ARGS>
    void Add(ARGS&&... args) {
        transforms_.emplace_back(std::make_unique<T>(std::forward<ARGS>(args)...));
    }

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    std::vector<std::unique_ptr<Transform>> transforms_;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_MANAGER_H_
