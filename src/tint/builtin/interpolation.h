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

#ifndef SRC_TINT_BUILTIN_INTERPOLATION_H_
#define SRC_TINT_BUILTIN_INTERPOLATION_H_

#include "src/tint/builtin/interpolation_sampling.h"
#include "src/tint/builtin/interpolation_type.h"

namespace tint::builtin {

/// The values of an `@interpolate` attribute
struct Interpolation {
    /// The first argument of a `@interpolate` attribute
    builtin::InterpolationType type = builtin::InterpolationType::kUndefined;
    /// The second argument of a `@interpolate` attribute
    builtin::InterpolationSampling sampling = builtin::InterpolationSampling::kUndefined;
};

}  // namespace tint::builtin

#endif  // SRC_TINT_BUILTIN_INTERPOLATION_H_
