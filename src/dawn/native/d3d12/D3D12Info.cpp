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

#include "dawn/native/d3d12/D3D12Info.h"

#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/native/d3d12/AdapterD3D12.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/PlatformFunctions.h"

namespace dawn::native::d3d12 {

ResultOrError<D3D12DeviceInfo> GatherDeviceInfo(const Adapter& adapter) {
    D3D12DeviceInfo info = {};

    // Newer builds replace D3D_FEATURE_DATA_ARCHITECTURE with
    // D3D_FEATURE_DATA_ARCHITECTURE1. However, D3D_FEATURE_DATA_ARCHITECTURE can be used
    // for backwards compat.
    // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ne-d3d12-d3d12_feature
    D3D12_FEATURE_DATA_ARCHITECTURE arch = {};
    DAWN_TRY(CheckHRESULT(
        adapter.GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &arch, sizeof(arch)),
        "ID3D12Device::CheckFeatureSupport"));

    info.isUMA = arch.UMA;

    D3D12_FEATURE_DATA_D3D12_OPTIONS featureOptions = {};
    DAWN_TRY(CheckHRESULT(adapter.GetDevice()->CheckFeatureSupport(
                              D3D12_FEATURE_D3D12_OPTIONS, &featureOptions, sizeof(featureOptions)),
                          "ID3D12Device::CheckFeatureSupport"));
    info.resourceHeapTier = featureOptions.ResourceHeapTier;

    D3D12_FEATURE_DATA_D3D12_OPTIONS2 featureOptions2 = {};
    if (SUCCEEDED(adapter.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS2, &featureOptions2, sizeof(featureOptions2)))) {
        info.programmableSamplePositionsTier = featureOptions2.ProgrammableSamplePositionsTier;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS3 featureOptions3 = {};
    if (SUCCEEDED(adapter.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS3, &featureOptions3, sizeof(featureOptions3)))) {
        info.supportsCastingFullyTypedFormat = featureOptions3.CastingFullyTypedFormatSupported;
    }

    // Used to share resources cross-API. If we query CheckFeatureSupport for
    // D3D12_FEATURE_D3D12_OPTIONS4 successfully, then we can use cross-API sharing.
    info.supportsSharedResourceCapabilityTier1 = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS4 featureOptions4 = {};
    if (SUCCEEDED(adapter.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS4, &featureOptions4, sizeof(featureOptions4)))) {
        // Tier 1 support additionally enables the NV12 format. Since only the NV12 format
        // is used by Dawn, check for Tier 1.
        if (featureOptions4.SharedResourceCompatibilityTier >=
            D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_1) {
            info.supportsSharedResourceCapabilityTier1 = true;
        }
    }

    // Windows builds 1809 and above can use the D3D12 render pass API. If we query
    // CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS5 successfully, then we can use
    // the render pass API.
    info.supportsRenderPass = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureOptions5 = {};
    if (SUCCEEDED(adapter.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS5, &featureOptions5, sizeof(featureOptions5)))) {
        // Performance regressions been observed when using a render pass on Intel graphics
        // with RENDER_PASS_TIER_1 available, so fall back to a software emulated render
        // pass on these platforms.
        if (featureOptions5.RenderPassesTier < D3D12_RENDER_PASS_TIER_1 ||
            !gpu_info::IsIntel(adapter.GetVendorId())) {
            info.supportsRenderPass = true;
        }
    }

    D3D12_FEATURE_DATA_SHADER_MODEL knownShaderModels[] = {
        {D3D_SHADER_MODEL_6_4}, {D3D_SHADER_MODEL_6_3}, {D3D_SHADER_MODEL_6_2},
        {D3D_SHADER_MODEL_6_1}, {D3D_SHADER_MODEL_6_0}, {D3D_SHADER_MODEL_5_1}};
    uint32_t driverShaderModel = 0;
    for (D3D12_FEATURE_DATA_SHADER_MODEL shaderModel : knownShaderModels) {
        if (SUCCEEDED(adapter.GetDevice()->CheckFeatureSupport(
                D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))) {
            driverShaderModel = shaderModel.HighestShaderModel;
            break;
        }
    }

    if (driverShaderModel < D3D_SHADER_MODEL_5_1) {
        return DAWN_INTERNAL_ERROR("Driver doesn't support Shader Model 5.1 or higher");
    }

    // D3D_SHADER_MODEL is encoded as 0xMm with M the major version and m the minor version
    ASSERT(driverShaderModel <= 0xFF);
    uint32_t shaderModelMajor = (driverShaderModel & 0xF0) >> 4;
    uint32_t shaderModelMinor = (driverShaderModel & 0xF);

    ASSERT(shaderModelMajor < 10);
    ASSERT(shaderModelMinor < 10);
    info.shaderModel = 10 * shaderModelMajor + shaderModelMinor;

    // Profiles are always <stage>s_<minor>_<major> so we build the s_<minor>_major and add
    // it to each of the stage's suffix.
    std::wstring profileSuffix = L"s_M_n";
    profileSuffix[2] = wchar_t('0' + shaderModelMajor);
    profileSuffix[4] = wchar_t('0' + shaderModelMinor);

    info.shaderProfiles[SingleShaderStage::Vertex] = L"v" + profileSuffix;
    info.shaderProfiles[SingleShaderStage::Fragment] = L"p" + profileSuffix;
    info.shaderProfiles[SingleShaderStage::Compute] = L"c" + profileSuffix;

    info.supportsShaderF16 =
        driverShaderModel >= D3D_SHADER_MODEL_6_2 && featureOptions4.Native16BitShaderOpsSupported;

    info.supportsDP4a = driverShaderModel >= D3D_SHADER_MODEL_6_4;

    return std::move(info);
}

}  // namespace dawn::native::d3d12
