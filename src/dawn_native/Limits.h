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

#ifndef DAWNNATIVE_LIMITS_H_
#define DAWNNATIVE_LIMITS_H_

#include "dawn_native/Error.h"
#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    struct CombinedLimits {
        wgpu::Limits v1;
    };

    // Populate |limits| with the default limits.
    void GetDefaultLimits(wgpu::Limits* limits);

    // Returns a copy of |limits| where all undefined values are replaced
    // with their defaults.
    wgpu::Limits ReifyDefaultLimits(const wgpu::Limits& limits);

    // Validate that |requiredLimits| are no better than |supportedLimits|.
    MaybeError ValidateLimits(const wgpu::Limits& supportedLimits,
                              const wgpu::Limits& requiredLimits);

}  // namespace dawn_native

#endif  // DAWNNATIVE_LIMITS_H_
