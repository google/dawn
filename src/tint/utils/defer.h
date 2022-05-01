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

#ifndef SRC_TINT_UTILS_DEFER_H_
#define SRC_TINT_UTILS_DEFER_H_

#include <utility>

#include "src/tint/utils/concat.h"

namespace tint::utils {

/// Defer executes a function or function like object when it is destructed.
template <typename F>
class Defer {
  public:
    /// Constructor
    /// @param f the function to call when the Defer is destructed
    explicit Defer(F&& f) : f_(std::move(f)) {}

    /// Move constructor
    Defer(Defer&&) = default;

    /// Destructor
    /// Calls the deferred function
    ~Defer() { f_(); }

  private:
    Defer(const Defer&) = delete;
    Defer& operator=(const Defer&) = delete;

    F f_;
};

/// Constructor
/// @param f the function to call when the Defer is destructed
template <typename F>
inline Defer<F> MakeDefer(F&& f) {
    return Defer<F>(std::forward<F>(f));
}

}  // namespace tint::utils

/// TINT_DEFER(S) executes the statement(s) `S` when exiting the current lexical
/// scope.
#define TINT_DEFER(S) \
    auto TINT_CONCAT(tint_defer_, __COUNTER__) = ::tint::utils::MakeDefer([&] { S; })

#endif  // SRC_TINT_UTILS_DEFER_H_
