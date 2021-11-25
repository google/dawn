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

#ifndef DAWNNATIVE_FEATURES_H_
#define DAWNNATIVE_FEATURES_H_

#include <bitset>
#include <unordered_map>
#include <vector>

#include "dawn_native/DawnNative.h"

namespace dawn_native {

    enum class Feature {
        TextureCompressionBC,
        TextureCompressionETC2,
        TextureCompressionASTC,
        ShaderFloat16,
        PipelineStatisticsQuery,
        TimestampQuery,
        DepthClamping,
        Depth24UnormStencil8,
        Depth32FloatStencil8,

        // Dawn-specific
        DawnInternalUsages,
        MultiPlanarFormats,

        EnumCount,
        InvalidEnum = EnumCount,
        FeatureMin = TextureCompressionBC,
    };

    // A wrapper of the bitset to store if an feature is enabled or not. This wrapper provides the
    // convenience to convert the enums of enum class Feature to the indices of a bitset.
    struct FeaturesSet {
        std::bitset<static_cast<size_t>(Feature::EnumCount)> featuresBitSet;

        void EnableFeature(Feature feature);
        bool IsEnabled(Feature feature) const;
        std::vector<const char*> GetEnabledFeatureNames() const;
        void InitializeDeviceProperties(WGPUDeviceProperties* properties) const;
    };

    const char* FeatureEnumToName(Feature feature);

    class FeaturesInfo {
      public:
        FeaturesInfo();

        // Used to query the details of an feature. Return nullptr if featureName is not a valid
        // name of an feature supported in Dawn
        const FeatureInfo* GetFeatureInfo(const char* featureName) const;
        Feature FeatureNameToEnum(const char* featureName) const;
        FeaturesSet FeatureNamesToFeaturesSet(
            const std::vector<const char*>& requiredFeatures) const;

      private:
        std::unordered_map<std::string, Feature> mFeatureNameToEnumMap;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_FEATURES_H_
