// Copyright 2023 The Dawn Authors
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

#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::platform::metrics {

DawnHistogramTimer::DawnHistogramTimer(Platform* platform)
    : mPlatform(platform),
      mConstructed(mPlatform != nullptr ? mPlatform->MonotonicallyIncreasingTime() : 0) {}

void DawnHistogramTimer::RecordMicroseconds(const char* name) {
    if (mPlatform == nullptr || name == nullptr || mConstructed == 0) {
        return;
    }
    // TODO(dawn:1934) Unify the constants when/if possible.
    int elapsedUS =
        static_cast<int>((mPlatform->MonotonicallyIncreasingTime() - mConstructed) * 1'000'000.0);
    mPlatform->HistogramCustomCountsHPC(name, elapsedUS, 1, 1'000'000, 50);
}

void DawnHistogramTimer::Reset() {
    if (mPlatform == nullptr) {
        return;
    }
    mConstructed = mPlatform->MonotonicallyIncreasingTime();
}

}  // namespace dawn::platform::metrics
