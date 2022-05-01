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

#include "dawn/native/Features.h"

#include <array>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"

namespace dawn::native {
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
       "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
       "dawn_internal_usages.md"},
      &WGPUDeviceProperties::dawnInternalUsages},
     {Feature::MultiPlanarFormats,
      {"multiplanar-formats", "Import and use multi-planar texture formats with per plane views",
       "https://bugs.chromium.org/p/dawn/issues/detail?id=551"},
      &WGPUDeviceProperties::multiPlanarFormats},
     {Feature::DawnNative,
      {"dawn-native", "WebGPU is running on top of dawn_native.",
       "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
       "dawn_native.md"},
      &WGPUDeviceProperties::dawnNative}}};

Feature FromAPIFeature(wgpu::FeatureName feature) {
    switch (feature) {
        case wgpu::FeatureName::Undefined:
            return Feature::InvalidEnum;

        case wgpu::FeatureName::TimestampQuery:
            return Feature::TimestampQuery;
        case wgpu::FeatureName::PipelineStatisticsQuery:
            return Feature::PipelineStatisticsQuery;
        case wgpu::FeatureName::TextureCompressionBC:
            return Feature::TextureCompressionBC;
        case wgpu::FeatureName::TextureCompressionETC2:
            return Feature::TextureCompressionETC2;
        case wgpu::FeatureName::TextureCompressionASTC:
            return Feature::TextureCompressionASTC;
        case wgpu::FeatureName::DepthClamping:
            return Feature::DepthClamping;
        case wgpu::FeatureName::Depth24UnormStencil8:
            return Feature::Depth24UnormStencil8;
        case wgpu::FeatureName::Depth32FloatStencil8:
            return Feature::Depth32FloatStencil8;
        case wgpu::FeatureName::DawnShaderFloat16:
            return Feature::ShaderFloat16;
        case wgpu::FeatureName::DawnInternalUsages:
            return Feature::DawnInternalUsages;
        case wgpu::FeatureName::DawnMultiPlanarFormats:
            return Feature::MultiPlanarFormats;
        case wgpu::FeatureName::DawnNative:
            return Feature::DawnNative;

        case wgpu::FeatureName::IndirectFirstInstance:
            return Feature::InvalidEnum;
    }
    return Feature::InvalidEnum;
}

wgpu::FeatureName ToAPIFeature(Feature feature) {
    switch (feature) {
        case Feature::TextureCompressionBC:
            return wgpu::FeatureName::TextureCompressionBC;
        case Feature::TextureCompressionETC2:
            return wgpu::FeatureName::TextureCompressionETC2;
        case Feature::TextureCompressionASTC:
            return wgpu::FeatureName::TextureCompressionASTC;
        case Feature::PipelineStatisticsQuery:
            return wgpu::FeatureName::PipelineStatisticsQuery;
        case Feature::TimestampQuery:
            return wgpu::FeatureName::TimestampQuery;
        case Feature::DepthClamping:
            return wgpu::FeatureName::DepthClamping;
        case Feature::Depth24UnormStencil8:
            return wgpu::FeatureName::Depth24UnormStencil8;
        case Feature::Depth32FloatStencil8:
            return wgpu::FeatureName::Depth32FloatStencil8;
        case Feature::ShaderFloat16:
            return wgpu::FeatureName::DawnShaderFloat16;
        case Feature::DawnInternalUsages:
            return wgpu::FeatureName::DawnInternalUsages;
        case Feature::MultiPlanarFormats:
            return wgpu::FeatureName::DawnMultiPlanarFormats;
        case Feature::DawnNative:
            return wgpu::FeatureName::DawnNative;

        case Feature::EnumCount:
            break;
    }
    UNREACHABLE();
}

}  // anonymous namespace

void FeaturesSet::EnableFeature(Feature feature) {
    ASSERT(feature != Feature::InvalidEnum);
    const size_t featureIndex = static_cast<size_t>(feature);
    featuresBitSet.set(featureIndex);
}

void FeaturesSet::EnableFeature(wgpu::FeatureName feature) {
    EnableFeature(FromAPIFeature(feature));
}

bool FeaturesSet::IsEnabled(Feature feature) const {
    ASSERT(feature != Feature::InvalidEnum);
    const size_t featureIndex = static_cast<size_t>(feature);
    return featuresBitSet[featureIndex];
}

bool FeaturesSet::IsEnabled(wgpu::FeatureName feature) const {
    Feature f = FromAPIFeature(feature);
    return f != Feature::InvalidEnum && IsEnabled(f);
}

size_t FeaturesSet::EnumerateFeatures(wgpu::FeatureName* features) const {
    for (uint32_t i : IterateBitSet(featuresBitSet)) {
        wgpu::FeatureName feature = ToAPIFeature(static_cast<Feature>(i));
        if (features != nullptr) {
            *features = feature;
            features += 1;
        }
    }
    return featuresBitSet.count();
}

std::vector<const char*> FeaturesSet::GetEnabledFeatureNames() const {
    std::vector<const char*> enabledFeatureNames(featuresBitSet.count());

    uint32_t index = 0;
    for (uint32_t i : IterateBitSet(featuresBitSet)) {
        Feature feature = static_cast<Feature>(i);
        ASSERT(feature != Feature::InvalidEnum);

        const FeatureEnumAndInfo& featureNameAndInfo = kFeatureNameAndInfoList[i];
        ASSERT(featureNameAndInfo.feature == feature);

        enabledFeatureNames[index] = featureNameAndInfo.info.name;
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

wgpu::FeatureName FeatureEnumToAPIFeature(Feature feature) {
    ASSERT(feature != Feature::InvalidEnum);
    return ToAPIFeature(feature);
}

FeaturesInfo::FeaturesInfo() {
    for (size_t index = 0; index < kFeatureNameAndInfoList.size(); ++index) {
        const FeatureEnumAndInfo& featureNameAndInfo = kFeatureNameAndInfoList[index];
        ASSERT(index == static_cast<size_t>(featureNameAndInfo.feature));
        mFeatureNameToEnumMap[featureNameAndInfo.info.name] = featureNameAndInfo.feature;
    }
}

const FeatureInfo* FeaturesInfo::GetFeatureInfo(wgpu::FeatureName feature) const {
    Feature f = FromAPIFeature(feature);
    if (f == Feature::InvalidEnum) {
        return nullptr;
    }
    return &kFeatureNameAndInfoList[static_cast<size_t>(f)].info;
}

Feature FeaturesInfo::FeatureNameToEnum(const char* featureName) const {
    ASSERT(featureName);

    const auto& iter = mFeatureNameToEnumMap.find(featureName);
    if (iter != mFeatureNameToEnumMap.cend()) {
        return kFeatureNameAndInfoList[static_cast<size_t>(iter->second)].feature;
    }

    // TODO(dawn:550): Remove this fallback logic when Chromium is updated.
    constexpr std::array<std::pair<const char*, const char*>, 6> kReplacementsForDeprecatedNames = {
        {
            {"texture_compression_bc", "texture-compression-bc"},
            {"depth_clamping", "depth-clamping"},
            {"pipeline_statistics_query", "pipeline-statistics-query"},
            {"shader_float16", "shader-float16"},
            {"timestamp_query", "timestamp-query"},
            {"multiplanar_formats", "multiplanar-formats"},
        }};
    for (const auto& [name, replacement] : kReplacementsForDeprecatedNames) {
        if (strcmp(featureName, name) == 0) {
            return FeatureNameToEnum(replacement);
        }
    }

    return Feature::InvalidEnum;
}

wgpu::FeatureName FeaturesInfo::FeatureNameToAPIEnum(const char* featureName) const {
    Feature f = FeatureNameToEnum(featureName);
    if (f != Feature::InvalidEnum) {
        return ToAPIFeature(f);
    }
    // Pass something invalid.
    return static_cast<wgpu::FeatureName>(-1);
}

}  // namespace dawn::native
