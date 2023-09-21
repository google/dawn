// Copyright 2019 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_FEATURES_H_
#define SRC_DAWN_NATIVE_FEATURES_H_

#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

#include "dawn/common/ityp_bitset.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Features_autogen.h"
#include "dawn/webgpu_cpp.h"

namespace dawn::native {

enum class FeatureLevel { Compatibility, Core };

extern const ityp::array<Feature, FeatureInfo, kEnumCount<Feature>> kFeatureNameAndInfoList;

wgpu::FeatureName ToAPI(Feature feature);
Feature FromAPI(wgpu::FeatureName feature);

// A wrapper of the bitset to store if an feature is enabled or not. This wrapper provides the
// convenience to convert the enums of enum class Feature to the indices of a bitset.
struct FeaturesSet {
    ityp::bitset<Feature, kEnumCount<Feature>> featuresBitSet;

    void EnableFeature(Feature feature);
    void EnableFeature(wgpu::FeatureName feature);
    bool IsEnabled(Feature feature) const;
    bool IsEnabled(wgpu::FeatureName feature) const;
    // Returns |count|, the number of features. Writes out all |count| values if |features| is
    // non-null.
    size_t EnumerateFeatures(wgpu::FeatureName* features) const;
    std::vector<const char*> GetEnabledFeatureNames() const;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_FEATURES_H_
