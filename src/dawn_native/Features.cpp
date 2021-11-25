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

#include <array>

#include "common/Assert.h"
#include "common/BitSetIterator.h"
#include "dawn_native/Features.h"

namespace dawn_native {
    namespace {

        struct FeatureEnumAndInfo {
            Feature feature;
            FeatureInfo info;
            bool WGPUDeviceProperties::*memberInWGPUDeviceProperties;
        };

        using FeatureEnumAndInfoList =
            std::array<FeatureEnumAndInfo, static_cast<size_t>(Feature::EnumCount)>;

        static constexpr FeatureEnumAndInfoList kFeatureNameAndInfoList = {
            {{Feature::TextureCompressionBC,
              {"texture-compression-bc", "Support Block Compressed (BC) texture formats",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=42"},
              &WGPUDeviceProperties::textureCompressionBC},
             {Feature::TextureCompressionETC2,
              {"texture-compression-etc2",
               "Support Ericsson Texture Compressed (ETC2/EAC) texture "
               "formats",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=955"},
              &WGPUDeviceProperties::textureCompressionETC2},
             {Feature::TextureCompressionASTC,
              {"texture-compression-astc",
               "Support Adaptable Scalable Texture Compressed (ASTC) "
               "texture formats",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=955"},
              &WGPUDeviceProperties::textureCompressionASTC},
             {Feature::ShaderFloat16,
              {"shader-float16",
               "Support 16bit float arithmetic and declarations in uniform and storage buffers",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=426"},
              &WGPUDeviceProperties::shaderFloat16},
             {Feature::PipelineStatisticsQuery,
              {"pipeline-statistics-query", "Support Pipeline Statistics Query",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=434"},
              &WGPUDeviceProperties::pipelineStatisticsQuery},
             {Feature::TimestampQuery,
              {"timestamp-query", "Support Timestamp Query",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=434"},
              &WGPUDeviceProperties::timestampQuery},
             {Feature::DepthClamping,
              {"depth-clamping", "Clamp depth to [0, 1] in NDC space instead of clipping",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=716"},
              &WGPUDeviceProperties::depthClamping},
             {Feature::Depth24UnormStencil8,
              {"depth24unorm-stencil8", "Support depth24unorm-stencil8 texture format",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=690"},
              &WGPUDeviceProperties::depth24UnormStencil8},
             {Feature::Depth32FloatStencil8,
              {"depth32float-stencil8", "Support depth32float-stencil8 texture format",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=690"},
              &WGPUDeviceProperties::depth32FloatStencil8},
             {Feature::DawnInternalUsages,
              {"dawn-internal-usages",
               "Add internal usages to resources to affect how the texture is allocated, but not "
               "frontend validation. Other internal commands may access this usage.",
               "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/features/"
               "dawn_internal_usages.md"},
              &WGPUDeviceProperties::dawnInternalUsages},
             {Feature::MultiPlanarFormats,
              {"multiplanar-formats",
               "Import and use multi-planar texture formats with per plane views",
               "https://bugs.chromium.org/p/dawn/issues/detail?id=551"},
              &WGPUDeviceProperties::multiPlanarFormats}}};

    }  // anonymous namespace

    void FeaturesSet::EnableFeature(Feature feature) {
        ASSERT(feature != Feature::InvalidEnum);
        const size_t featureIndex = static_cast<size_t>(feature);
        featuresBitSet.set(featureIndex);
    }

    bool FeaturesSet::IsEnabled(Feature feature) const {
        ASSERT(feature != Feature::InvalidEnum);
        const size_t featureIndex = static_cast<size_t>(feature);
        return featuresBitSet[featureIndex];
    }

    std::vector<const char*> FeaturesSet::GetEnabledFeatureNames() const {
        std::vector<const char*> enabledFeatureNames(featuresBitSet.count());

        uint32_t index = 0;
        for (uint32_t i : IterateBitSet(featuresBitSet)) {
            const char* featureName = FeatureEnumToName(static_cast<Feature>(i));
            enabledFeatureNames[index] = featureName;
            ++index;
        }
        return enabledFeatureNames;
    }

    void FeaturesSet::InitializeDeviceProperties(WGPUDeviceProperties* properties) const {
        ASSERT(properties != nullptr);

        for (uint32_t i : IterateBitSet(featuresBitSet)) {
            properties->*(kFeatureNameAndInfoList[i].memberInWGPUDeviceProperties) = true;
        }
    }

    const char* FeatureEnumToName(Feature feature) {
        ASSERT(feature != Feature::InvalidEnum);

        const FeatureEnumAndInfo& featureNameAndInfo =
            kFeatureNameAndInfoList[static_cast<size_t>(feature)];
        ASSERT(featureNameAndInfo.feature == feature);
        return featureNameAndInfo.info.name;
    }

    FeaturesInfo::FeaturesInfo() {
        for (size_t index = 0; index < kFeatureNameAndInfoList.size(); ++index) {
            const FeatureEnumAndInfo& featureNameAndInfo = kFeatureNameAndInfoList[index];
            ASSERT(index == static_cast<size_t>(featureNameAndInfo.feature));
            mFeatureNameToEnumMap[featureNameAndInfo.info.name] = featureNameAndInfo.feature;
        }
    }

    const FeatureInfo* FeaturesInfo::GetFeatureInfo(const char* featureName) const {
        ASSERT(featureName);

        const auto& iter = mFeatureNameToEnumMap.find(featureName);
        if (iter != mFeatureNameToEnumMap.cend()) {
            return &kFeatureNameAndInfoList[static_cast<size_t>(iter->second)].info;
        }
        return nullptr;
    }

    Feature FeaturesInfo::FeatureNameToEnum(const char* featureName) const {
        ASSERT(featureName);

        const auto& iter = mFeatureNameToEnumMap.find(featureName);
        if (iter != mFeatureNameToEnumMap.cend()) {
            return kFeatureNameAndInfoList[static_cast<size_t>(iter->second)].feature;
        }

        // TODO(dawn:550): Remove this fallback logic when Chromium is updated.
        constexpr std::array<std::pair<const char*, const char*>, 6>
            kReplacementsForDeprecatedNames = {{
                {"texture_compression_bc", "texture-compression-bc"},
                {"depth_clamping", "depth-clamping"},
                {"pipeline_statistics_query", "pipeline-statistics-query"},
                {"shader_float16", "shader-float16"},
                {"timestamp_query", "timestamp-query"},
                {"multiplanar_formats", "multiplanar-formats"},
            }};
        for (const auto& replacement : kReplacementsForDeprecatedNames) {
            if (strcmp(featureName, replacement.first) == 0) {
                return FeatureNameToEnum(replacement.second);
            }
        }

        return Feature::InvalidEnum;
    }

    FeaturesSet FeaturesInfo::FeatureNamesToFeaturesSet(
        const std::vector<const char*>& requiredFeatures) const {
        FeaturesSet featuresSet;

        for (const char* featureName : requiredFeatures) {
            Feature featureEnum = FeatureNameToEnum(featureName);
            ASSERT(featureEnum != Feature::InvalidEnum);
            featuresSet.EnableFeature(featureEnum);
        }
        return featuresSet;
    }

}  // namespace dawn_native
