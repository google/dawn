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

#include "dawn_native/d3d12/D3D12Info.h"

#include "dawn_native/d3d12/AdapterD3D12.h"
#include "dawn_native/d3d12/BackendD3D12.h"
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/PlatformFunctions.h"

namespace dawn_native { namespace d3d12 {

    ResultOrError<D3D12DeviceInfo> GatherDeviceInfo(const Adapter& adapter) {
        D3D12DeviceInfo info = {};

        // Newer builds replace D3D_FEATURE_DATA_ARCHITECTURE with
        // D3D_FEATURE_DATA_ARCHITECTURE1. However, D3D_FEATURE_DATA_ARCHITECTURE can be used
        // for backwards compat.
        // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ne-d3d12-d3d12_feature
        D3D12_FEATURE_DATA_ARCHITECTURE arch = {};
        DAWN_TRY(CheckHRESULT(adapter.GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE,
                                                                       &arch, sizeof(arch)),
                              "ID3D12Device::CheckFeatureSupport"));

        info.isUMA = arch.UMA;

        D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
        DAWN_TRY(CheckHRESULT(adapter.GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS,
                                                                       &options, sizeof(options)),
                              "ID3D12Device::CheckFeatureSupport"));

        info.resourceHeapTier = options.ResourceHeapTier;

        // Windows builds 1809 and above can use the D3D12 render pass API. If we query
        // CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS5 successfully, then we can use
        // the render pass API.
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureOptions5 = {};
        if (SUCCEEDED(adapter.GetDevice()->CheckFeatureSupport(
                D3D12_FEATURE_D3D12_OPTIONS5, &featureOptions5, sizeof(featureOptions5)))) {
            info.supportsRenderPass = true;
        } else {
            info.supportsRenderPass = false;
        }

        return info;
    }
}}  // namespace dawn_native::d3d12
