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

#ifndef SRC_TINT_LANG_CORE_IR_LOCATION_H_
#define SRC_TINT_LANG_CORE_IR_LOCATION_H_

#include <optional>

#include "src/tint/lang/core/interpolation.h"

namespace tint::ir {

/// A function parameter in the IR.
struct Location {
    /// The location value
    uint32_t value = 0;
    /// The interpolation settings
    std::optional<core::Interpolation> interpolation;
};

}  // namespace tint::ir

#endif  // SRC_TINT_LANG_CORE_IR_LOCATION_H_
