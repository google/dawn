// Copyright 2020 The Dawn Authors
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

#include "dawn_native/d3d12/ResidencyManagerD3D12.h"

#include "dawn_native/d3d12/AdapterD3D12.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/Forward.h"
#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

    ResidencyManager::ResidencyManager(Device* device)
        : mDevice(device),
          mResidencyManagementEnabled(
              device->IsToggleEnabled(Toggle::UseD3D12ResidencyManagement)) {
        UpdateVideoMemoryInfo();
    }

    // Allows an application component external to Dawn to cap Dawn's residency budget to prevent
    // competition for device local memory. Returns the amount of memory reserved, which may be less
    // that the requested reservation when under pressure.
    uint64_t ResidencyManager::SetExternalMemoryReservation(uint64_t requestedReservationSize) {
        mVideoMemoryInfo.externalRequest = requestedReservationSize;
        UpdateVideoMemoryInfo();
        return mVideoMemoryInfo.externalReservation;
    }

    void ResidencyManager::UpdateVideoMemoryInfo() {
        if (!mResidencyManagementEnabled) {
            return;
        }

        DXGI_QUERY_VIDEO_MEMORY_INFO queryVideoMemoryInfo;
        ToBackend(mDevice->GetAdapter())
            ->GetHardwareAdapter()
            ->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &queryVideoMemoryInfo);

        // The video memory budget provided by QueryVideoMemoryInfo is defined by the operating
        // system, and may be lower than expected in certain scenarios. Under memory pressure, we
        // cap the external reservation to half the available budget, which prevents the external
        // component from consuming a disproportionate share of memory and ensures that Dawn can
        // continue to make forward progress. Note the choice to halve memory is arbitrarily chosen
        // and subject to future experimentation.
        mVideoMemoryInfo.externalReservation =
            std::min(queryVideoMemoryInfo.Budget / 2, mVideoMemoryInfo.externalReservation);

        // We cap Dawn's budget to 95% of the provided budget. Leaving some budget unused
        // decreases fluctuations in the operating-system-defined budget, which improves stability
        // for both Dawn and other applications on the system. Note the value of 95% is arbitrarily
        // chosen and subject to future experimentation.
        static constexpr float kBudgetCap = 0.95;
        mVideoMemoryInfo.dawnBudget =
            (queryVideoMemoryInfo.Budget - mVideoMemoryInfo.externalReservation) * kBudgetCap;
        mVideoMemoryInfo.dawnUsage =
            queryVideoMemoryInfo.CurrentUsage - mVideoMemoryInfo.externalReservation;
    }
}}  // namespace dawn_native::d3d12