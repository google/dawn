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

#include "dawn/wire/SupportedFeatures.h"

namespace dawn::wire {

// Note: Upon updating this list, please also update serialization/deserialization
// of limit structs on Adapter/Device initialization.
bool IsFeatureSupported(WGPUFeatureName feature) {
    switch (feature) {
        case WGPUFeatureName_Undefined:
        case WGPUFeatureName_Force32:
        case WGPUFeatureName_DawnNative:
        case WGPUFeatureName_DawnShaderFloat16:  // Deprecated
        case WGPUFeatureName_ImplicitDeviceSynchronization:
        case WGPUFeatureName_SurfaceCapabilities:
        case WGPUFeatureName_D3D11MultithreadProtected:
            return false;
        case WGPUFeatureName_Depth32FloatStencil8:
        case WGPUFeatureName_TimestampQuery:
        case WGPUFeatureName_TimestampQueryInsidePasses:
        case WGPUFeatureName_PipelineStatisticsQuery:
        case WGPUFeatureName_TextureCompressionBC:
        case WGPUFeatureName_TextureCompressionETC2:
        case WGPUFeatureName_TextureCompressionASTC:
        case WGPUFeatureName_IndirectFirstInstance:
        case WGPUFeatureName_DepthClipControl:
        case WGPUFeatureName_DawnInternalUsages:
        case WGPUFeatureName_DawnMultiPlanarFormats:
        case WGPUFeatureName_ChromiumExperimentalDp4a:
        case WGPUFeatureName_ShaderF16:
        case WGPUFeatureName_RG11B10UfloatRenderable:
        case WGPUFeatureName_BGRA8UnormStorage:
        case WGPUFeatureName_TransientAttachments:
        case WGPUFeatureName_Float32Filterable:
        case WGPUFeatureName_MSAARenderToSingleSampled:
        case WGPUFeatureName_DualSourceBlending:
        case WGPUFeatureName_ANGLETextureSharing:
            return true;
    }

    // Catch-all, for unsupported features.
    // "default:" is not used so we get compiler errors for
    // newly added, unhandled features, but still catch completely
    // unknown enums.
    return false;
}

}  // namespace dawn::wire
