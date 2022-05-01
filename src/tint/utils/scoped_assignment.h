
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

#ifndef SRC_TINT_UTILS_SCOPED_ASSIGNMENT_H_
#define SRC_TINT_UTILS_SCOPED_ASSIGNMENT_H_

#include <type_traits>

#include "src/tint/utils/concat.h"

namespace tint::utils {

/// Helper class that temporarily assigns a value to a variable for the lifetime
/// of the ScopedAssignment object. Once the ScopedAssignment is destructed, the
/// original value is restored.
template <typename T>
class ScopedAssignment {
  public:
    /// Constructor
    /// @param var the variable to temporarily assign a new value to
    /// @param val the value to assign to `ref` for the lifetime of this
    /// ScopedAssignment.
    ScopedAssignment(T& var, T val) : ref_(var) {
        old_value_ = var;
        var = val;
    }

    /// Destructor
    /// Restores the original value of the variable.
    ~ScopedAssignment() { ref_ = old_value_; }

  private:
    ScopedAssignment(const ScopedAssignment&) = delete;
    ScopedAssignment& operator=(const ScopedAssignment&) = delete;

    T& ref_;
    T old_value_;
};

}  // namespace tint::utils

/// TINT_SCOPED_ASSIGNMENT(var, val) assigns `val` to `var`, and automatically
/// restores the original value of `var` when exiting the current lexical scope.
#define TINT_SCOPED_ASSIGNMENT(var, val)                                                 \
    ::tint::utils::ScopedAssignment<std::remove_reference_t<decltype(var)>> TINT_CONCAT( \
        tint_scoped_assignment_, __COUNTER__) {                                          \
        var, val                                                                         \
    }

#endif  // SRC_TINT_UTILS_SCOPED_ASSIGNMENT_H_
