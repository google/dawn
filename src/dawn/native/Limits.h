// Copyright 2021 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_LIMITS_H_
#define SRC_DAWN_NATIVE_LIMITS_H_

#include "dawn/native/Error.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

struct CombinedLimits {
    Limits v1;
};

// Populate |limits| with the default limits.
void GetDefaultLimits(Limits* limits);

// Returns a copy of |limits| where all undefined values are replaced
// with their defaults. Also clamps to the defaults if the provided limits
// are worse.
Limits ReifyDefaultLimits(const Limits& limits);

// Validate that |requiredLimits| are no better than |supportedLimits|.
MaybeError ValidateLimits(const Limits& supportedLimits, const Limits& requiredLimits);

// Returns a copy of |limits| where limit tiers are applied.
Limits ApplyLimitTiers(Limits limits);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_LIMITS_H_
