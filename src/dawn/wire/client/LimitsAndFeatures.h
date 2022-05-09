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

#ifndef SRC_DAWN_WIRE_CLIENT_LIMITSANDFEATURES_H_
#define SRC_DAWN_WIRE_CLIENT_LIMITSANDFEATURES_H_

#include <unordered_set>

#include "dawn/webgpu.h"

namespace dawn::wire::client {

class LimitsAndFeatures {
  public:
    LimitsAndFeatures();
    ~LimitsAndFeatures();

    bool GetLimits(WGPUSupportedLimits* limits) const;
    bool HasFeature(WGPUFeatureName feature) const;
    size_t EnumerateFeatures(WGPUFeatureName* features) const;

    void SetLimits(const WGPUSupportedLimits* limits);
    void SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount);

  private:
    WGPUSupportedLimits mLimits;
    std::unordered_set<WGPUFeatureName> mFeatures;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_LIMITSANDFEATURES_H_
