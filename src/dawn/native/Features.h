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
#include "dawn/webgpu_cpp.h"

namespace dawn::native {

enum class Feature {
    TextureCompressionBC,
    TextureCompressionETC2,
    TextureCompressionASTC,
    PipelineStatisticsQuery,
    TimestampQuery,
    TimestampQueryInsidePasses,
    DepthClipControl,
    Depth32FloatStencil8,
    ChromiumExperimentalDp4a,
    IndirectFirstInstance,
    ShaderF16,
    RG11B10UfloatRenderable,
    BGRA8UnormStorage,
    Float32Filterable,

    // Dawn-specific
    DawnInternalUsages,
    MultiPlanarFormats,
    DawnNative,
    ImplicitDeviceSynchronization,
    SurfaceCapabilities,
    TransientAttachments,
    MSAARenderToSingleSampled,
    DualSourceBlending,
    D3D11MultithreadProtected,
    ANGLETextureSharing,

    SharedTextureMemoryVkDedicatedAllocation,
    SharedTextureMemoryAHardwareBuffer,
    SharedTextureMemoryDmaBuf,
    SharedTextureMemoryOpaqueFD,
    SharedTextureMemoryZirconHandle,
    SharedTextureMemoryDXGISharedHandle,
    SharedTextureMemoryD3D11Texture2D,
    SharedTextureMemoryIOSurface,
    SharedTextureMemoryEGLImage,

    SharedFenceVkSemaphoreOpaqueFD,
    SharedFenceVkSemaphoreSyncFD,
    SharedFenceVkSemaphoreZirconHandle,
    SharedFenceDXGISharedHandle,
    SharedFenceMTLSharedEvent,

    EnumCount,
    InvalidEnum = EnumCount,
    FeatureMin = TextureCompressionBC,
};

// A wrapper of the bitset to store if an feature is enabled or not. This wrapper provides the
// convenience to convert the enums of enum class Feature to the indices of a bitset.
struct FeaturesSet {
    std::bitset<static_cast<size_t>(Feature::EnumCount)> featuresBitSet;

    void EnableFeature(Feature feature);
    void EnableFeature(wgpu::FeatureName feature);
    bool IsEnabled(Feature feature) const;
    bool IsEnabled(wgpu::FeatureName feature) const;
    // Returns |count|, the number of features. Writes out all |count| values if |features| is
    // non-null.
    size_t EnumerateFeatures(wgpu::FeatureName* features) const;
    std::vector<const char*> GetEnabledFeatureNames() const;
};

wgpu::FeatureName FeatureEnumToAPIFeature(Feature feature);

class FeaturesInfo {
  public:
    FeaturesInfo();
    ~FeaturesInfo();

    // Used to query the details of an feature. Return nullptr if featureName is not a valid
    // name of an feature supported in Dawn
    const FeatureInfo* GetFeatureInfo(wgpu::FeatureName feature) const;
    Feature FeatureNameToEnum(const char* featureName) const;
    wgpu::FeatureName FeatureNameToAPIEnum(const char* featureName) const;

  private:
    std::unordered_map<std::string, Feature> mFeatureNameToEnumMap;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_FEATURES_H_
