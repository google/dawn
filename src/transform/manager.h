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

#ifndef SRC_TRANSFORM_MANAGER_H_
#define SRC_TRANSFORM_MANAGER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/diagnostic/diagnostic.h"
#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// Manager for the provided passes. The passes will be execute in the
/// appended order. If any pass fails the manager will return immediately and
/// the error can be retrieved with the error() method.
class Manager : public Transform {
 public:
  /// Constructor
  Manager();
  ~Manager() override;

  /// Add pass to the manager
  /// @param transform the transform to append
  void append(std::unique_ptr<Transform> transform) {
    transforms_.push_back(std::move(transform));
  }

  /// Runs the transforms on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @returns the transformed program and diagnostics
  Output Run(const Program* program) override;

 private:
  std::vector<std::unique_ptr<Transform>> transforms_;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_MANAGER_H_
