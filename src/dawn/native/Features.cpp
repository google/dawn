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
};

using FeatureEnumAndInfoList =
    std::array<FeatureEnumAndInfo, static_cast<size_t>(Feature::EnumCount)>;

static constexpr FeatureEnumAndInfoList kFeatureNameAndInfoList = {{
    {Feature::TextureCompressionBC,
     {"texture-compression-bc", "Support Block Compressed (BC) texture formats",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=42", FeatureInfo::FeatureState::Stable}},
    {Feature::TextureCompressionETC2,
     {"texture-compression-etc2",
      "Support Ericsson Texture Compressed (ETC2/EAC) texture "
      "formats",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=955", FeatureInfo::FeatureState::Stable}},
    {Feature::TextureCompressionASTC,
     {"texture-compression-astc",
      "Support Adaptable Scalable Texture Compressed (ASTC) "
      "texture formats",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=955", FeatureInfo::FeatureState::Stable}},
    {Feature::PipelineStatisticsQuery,
     {"pipeline-statistics-query", "Support Pipeline Statistics Query",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=434",
      FeatureInfo::FeatureState::Experimental}},
    {Feature::TimestampQuery,
     {"timestamp-query", "Support Timestamp Query",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=434",
      FeatureInfo::FeatureState::Experimental}},
    {Feature::TimestampQueryInsidePasses,
     {"timestamp-query-inside-passes", "Support Timestamp Query inside render/compute pass",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=434",
      FeatureInfo::FeatureState::Experimental}},
    {Feature::DepthClipControl,
     {"depth-clip-control", "Disable depth clipping of primitives to the clip volume",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1178", FeatureInfo::FeatureState::Stable}},
    {Feature::Depth32FloatStencil8,
     {"depth32float-stencil8", "Support depth32float-stencil8 texture format",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=690", FeatureInfo::FeatureState::Stable}},
    {Feature::ChromiumExperimentalDp4a,
     {"chromium-experimental-dp4a", "Support experimental DP4a instructions in WGSL",
      "https://bugs.chromium.org/p/tint/issues/detail?id=1497",
      FeatureInfo::FeatureState::Experimental}},
    {Feature::IndirectFirstInstance,
     {"indirect-first-instance", "Support non-zero first instance values on indirect draw calls",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1197", FeatureInfo::FeatureState::Stable}},
    {Feature::ShaderF16,
     {"shader-f16", "Supports the \"enable f16;\" directive in WGSL",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1510",
      FeatureInfo::FeatureState::Experimental}},
    {Feature::RG11B10UfloatRenderable,
     {"rg11b10ufloat-renderable",
      "Allows the RENDER_ATTACHMENT usage on textures with format \"rg11b10ufloat\", and also "
      "allows textures of that format to be multisampled.",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1518", FeatureInfo::FeatureState::Stable}},
    {Feature::BGRA8UnormStorage,
     {"bgra8unorm-storage", "Allows the STORAGE usage on textures with format \"bgra8unorm\".",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1591", FeatureInfo::FeatureState::Stable}},
    {Feature::Float32Filterable,
     {"float32-filterable",
      "Allows textures with formats \"r32float\" \"rg32float\" and \"rgba32float\" to be filtered.",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1664",
      FeatureInfo::FeatureState::Experimental}},
    {Feature::DawnInternalUsages,
     {"dawn-internal-usages",
      "Add internal usages to resources to affect how the texture is allocated, but not "
      "frontend validation. Other internal commands may access this usage.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dawn_internal_usages.md",
      FeatureInfo::FeatureState::Stable}},
    {Feature::MultiPlanarFormats,
     {"multiplanar-formats", "Import and use multi-planar texture formats with per plane views",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=551", FeatureInfo::FeatureState::Stable}},
    {Feature::DawnNative,
     {"dawn-native", "WebGPU is running on top of dawn_native.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dawn_native.md",
      FeatureInfo::FeatureState::Stable}},
    {Feature::ImplicitDeviceSynchronization,
     {"implicit-device-sync",
      "Public API methods (except encoding) will have implicit device synchronization. So they "
      "will be safe to be used on multiple threads.",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1662", FeatureInfo::FeatureState::Stable}},
    {Feature::SurfaceCapabilities,
     {"surface-capabilities",
      "Support querying Surface's capabilities such as supported usage flags. This feature also "
      "enables swap chain to be created with usage other than RenderAttachment.",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1760", FeatureInfo::FeatureState::Stable}},
    {Feature::TransientAttachments,
     {"transient-attachments",
      "Support transient attachments that allow render pass operations to stay in tile memory, "
      "avoiding VRAM traffic and potentially avoiding VRAM allocation for the textures.",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1695", FeatureInfo::FeatureState::Stable}},
    {Feature::MSAARenderToSingleSampled,
     {"msaa-render-to-single-sampled",
      "Support multisampled rendering on single-sampled attachments efficiently.",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1710", FeatureInfo::FeatureState::Stable}},
    {Feature::DualSourceBlending,
     {"dual-source-blending",
      "Support dual source blending. Enables Src1, OneMinusSrc1, Src1Alpha, and OneMinusSrc1Alpha "
      "blend factors along with @index WGSL output attribute.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dual_source_blending.md",
      FeatureInfo::FeatureState::Experimental}},
    {Feature::D3D11MultithreadProtected,
     {"d3d11-multithread-protected",
      "Enable ID3D11Multithread protection for interop with external users of the D3D11 device.",
      "https://bugs.chromium.org/p/dawn/issues/detail?id=1927", FeatureInfo::FeatureState::Stable}},
    {Feature::ANGLETextureSharing,
     {"angle-texture-sharing",
      "Enable ANGLE texture sharing to allow the OpenGL ES backend to share textures by external "
      "OpenGL texture ID.",
      "https://chromium.googlesource.com/angle/angle/+/refs/heads/main/extensions/"
      "EGL_ANGLE_display_texture_share_group.txt",
      FeatureInfo::FeatureState::Stable}},
}};

Feature FromAPIFeature(wgpu::FeatureName feature) {
    switch (feature) {
        case wgpu::FeatureName::Undefined:
            return Feature::InvalidEnum;
        case wgpu::FeatureName::DawnShaderFloat16:
            // Deprecated.
            return Feature::InvalidEnum;

        case wgpu::FeatureName::TimestampQuery:
            return Feature::TimestampQuery;
        case wgpu::FeatureName::TimestampQueryInsidePasses:
            return Feature::TimestampQueryInsidePasses;
        case wgpu::FeatureName::PipelineStatisticsQuery:
            return Feature::PipelineStatisticsQuery;
        case wgpu::FeatureName::TextureCompressionBC:
            return Feature::TextureCompressionBC;
        case wgpu::FeatureName::TextureCompressionETC2:
            return Feature::TextureCompressionETC2;
        case wgpu::FeatureName::TextureCompressionASTC:
            return Feature::TextureCompressionASTC;
        case wgpu::FeatureName::DepthClipControl:
            return Feature::DepthClipControl;
        case wgpu::FeatureName::Depth32FloatStencil8:
            return Feature::Depth32FloatStencil8;
        case wgpu::FeatureName::IndirectFirstInstance:
            return Feature::IndirectFirstInstance;
        case wgpu::FeatureName::DawnInternalUsages:
            return Feature::DawnInternalUsages;
        case wgpu::FeatureName::DawnMultiPlanarFormats:
            return Feature::MultiPlanarFormats;
        case wgpu::FeatureName::DawnNative:
            return Feature::DawnNative;
        case wgpu::FeatureName::ChromiumExperimentalDp4a:
            return Feature::ChromiumExperimentalDp4a;
        case wgpu::FeatureName::ShaderF16:
            return Feature::ShaderF16;
        case wgpu::FeatureName::RG11B10UfloatRenderable:
            return Feature::RG11B10UfloatRenderable;
        case wgpu::FeatureName::BGRA8UnormStorage:
            return Feature::BGRA8UnormStorage;
        case wgpu::FeatureName::ImplicitDeviceSynchronization:
            return Feature::ImplicitDeviceSynchronization;
        case wgpu::FeatureName::SurfaceCapabilities:
            return Feature::SurfaceCapabilities;
        case wgpu::FeatureName::TransientAttachments:
            return Feature::TransientAttachments;
        case wgpu::FeatureName::Float32Filterable:
            return Feature::Float32Filterable;
        case wgpu::FeatureName::MSAARenderToSingleSampled:
            return Feature::MSAARenderToSingleSampled;
        case wgpu::FeatureName::DualSourceBlending:
            return Feature::DualSourceBlending;
        case wgpu::FeatureName::D3D11MultithreadProtected:
            return Feature::D3D11MultithreadProtected;
        case wgpu::FeatureName::ANGLETextureSharing:
            return Feature::ANGLETextureSharing;
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
        case Feature::TimestampQueryInsidePasses:
            return wgpu::FeatureName::TimestampQueryInsidePasses;
        case Feature::DepthClipControl:
            return wgpu::FeatureName::DepthClipControl;
        case Feature::Depth32FloatStencil8:
            return wgpu::FeatureName::Depth32FloatStencil8;
        case Feature::IndirectFirstInstance:
            return wgpu::FeatureName::IndirectFirstInstance;
        case Feature::DawnInternalUsages:
            return wgpu::FeatureName::DawnInternalUsages;
        case Feature::MultiPlanarFormats:
            return wgpu::FeatureName::DawnMultiPlanarFormats;
        case Feature::DawnNative:
            return wgpu::FeatureName::DawnNative;
        case Feature::ChromiumExperimentalDp4a:
            return wgpu::FeatureName::ChromiumExperimentalDp4a;
        case Feature::ShaderF16:
            return wgpu::FeatureName::ShaderF16;
        case Feature::RG11B10UfloatRenderable:
            return wgpu::FeatureName::RG11B10UfloatRenderable;
        case Feature::BGRA8UnormStorage:
            return wgpu::FeatureName::BGRA8UnormStorage;
        case Feature::ImplicitDeviceSynchronization:
            return wgpu::FeatureName::ImplicitDeviceSynchronization;
        case Feature::SurfaceCapabilities:
            return wgpu::FeatureName::SurfaceCapabilities;
        case Feature::TransientAttachments:
            return wgpu::FeatureName::TransientAttachments;
        case Feature::Float32Filterable:
            return wgpu::FeatureName::Float32Filterable;
        case Feature::MSAARenderToSingleSampled:
            return wgpu::FeatureName::MSAARenderToSingleSampled;
        case Feature::DualSourceBlending:
            return wgpu::FeatureName::DualSourceBlending;
        case Feature::D3D11MultithreadProtected:
            return wgpu::FeatureName::D3D11MultithreadProtected;
        case Feature::ANGLETextureSharing:
            return wgpu::FeatureName::ANGLETextureSharing;
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

FeaturesInfo::~FeaturesInfo() = default;

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
