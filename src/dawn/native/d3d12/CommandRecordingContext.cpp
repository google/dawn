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

#include "dawn/native/d3d12/CommandRecordingContext.h"

#include <profileapi.h>
#include <sysinfoapi.h>

#include <string>
#include <utility>

#include "dawn/native/d3d12/CommandAllocatorManager.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/HeapD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d12 {

void CommandRecordingContext::AddToSharedTextureList(Texture* texture) {
    ASSERT(IsOpen());
    mSharedTextures.insert(texture);
}

MaybeError CommandRecordingContext::Open(ID3D12Device* d3d12Device,
                                         CommandAllocatorManager* commandAllocationManager) {
    ASSERT(!IsOpen());
    ID3D12CommandAllocator* commandAllocator;
    DAWN_TRY_ASSIGN(commandAllocator, commandAllocationManager->ReserveCommandAllocator());
    if (mD3d12CommandList != nullptr) {
        MaybeError error = CheckHRESULT(mD3d12CommandList->Reset(commandAllocator, nullptr),
                                        "D3D12 resetting command list");
        if (error.IsError()) {
            mD3d12CommandList.Reset();
            DAWN_TRY(std::move(error));
        }
    } else {
        ComPtr<ID3D12GraphicsCommandList> d3d12GraphicsCommandList;
        DAWN_TRY(CheckHRESULT(
            d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator,
                                           nullptr, IID_PPV_ARGS(&d3d12GraphicsCommandList)),
            "D3D12 creating direct command list"));
        mD3d12CommandList = std::move(d3d12GraphicsCommandList);
        // Store a cast to ID3D12GraphicsCommandList4. This is required to use the D3D12 render
        // pass APIs introduced in Windows build 1809.
        mD3d12CommandList.As(&mD3d12CommandList4);
    }

    mIsOpen = true;
    mNeedsSubmit = false;

    return {};
}

MaybeError CommandRecordingContext::ExecuteCommandList(Device* device) {
    if (IsOpen()) {
        // Shared textures must be transitioned to common state after the last usage in order
        // for them to be used by other APIs like D3D11. We ensure this by transitioning to the
        // common state right before command list submission. TransitionUsageNow itself ensures
        // no unnecessary transitions happen if the resources is already in the common state.
        for (Texture* texture : mSharedTextures) {
            DAWN_TRY(texture->SynchronizeImportedTextureBeforeUse());
            texture->TrackAllUsageAndTransitionNow(this, D3D12_RESOURCE_STATE_COMMON);
        }

        MaybeError error =
            CheckHRESULT(mD3d12CommandList->Close(), "D3D12 closing pending command list");
        if (error.IsError()) {
            Release();
            DAWN_TRY(std::move(error));
        }
        DAWN_TRY(device->GetResidencyManager()->EnsureHeapsAreResident(mHeapsPendingUsage.data(),
                                                                       mHeapsPendingUsage.size()));

        if (device->IsToggleEnabled(Toggle::RecordDetailedTimingInTraceEvents)) {
            uint64_t gpuTimestamp;
            uint64_t cpuTimestamp;
            FILETIME fileTimeNonPrecise;
            SYSTEMTIME systemTimeNonPrecise;

            // Both supported since Windows 2000, have a accuracy of 1ms
            GetSystemTimeAsFileTime(&fileTimeNonPrecise);
            GetSystemTime(&systemTimeNonPrecise);
            // Query CPU and GPU timestamps at almost the same time
            device->GetCommandQueue()->GetClockCalibration(&gpuTimestamp, &cpuTimestamp);

            uint64_t gpuFrequency;
            uint64_t cpuFrequency;
            LARGE_INTEGER cpuFrequencyLargeInteger;
            device->GetCommandQueue()->GetTimestampFrequency(&gpuFrequency);
            QueryPerformanceFrequency(&cpuFrequencyLargeInteger);  // Supported since Windows 2000
            cpuFrequency = cpuFrequencyLargeInteger.QuadPart;

            std::string timingInfo = absl::StrFormat(
                "UTC Time: %u/%u/%u %02u:%02u:%02u.%03u, File Time: %u, CPU "
                "Timestamp: %u, GPU Timestamp: %u, CPU Tick Frequency: %u, GPU Tick Frequency: "
                "%u",
                systemTimeNonPrecise.wYear, systemTimeNonPrecise.wMonth, systemTimeNonPrecise.wDay,
                systemTimeNonPrecise.wHour, systemTimeNonPrecise.wMinute,
                systemTimeNonPrecise.wSecond, systemTimeNonPrecise.wMilliseconds,
                (static_cast<uint64_t>(fileTimeNonPrecise.dwHighDateTime) << 32) +
                    fileTimeNonPrecise.dwLowDateTime,
                cpuTimestamp, gpuTimestamp, cpuFrequency, gpuFrequency);

            TRACE_EVENT_INSTANT1(
                device->GetPlatform(), General,
                "d3d12::CommandRecordingContext::ExecuteCommandList Detailed Timing", "Timing",
                timingInfo.c_str());
        }

        ID3D12CommandList* d3d12CommandList = GetCommandList();
        device->GetCommandQueue()->ExecuteCommandLists(1, &d3d12CommandList);

        for (Texture* texture : mSharedTextures) {
            DAWN_TRY(texture->SynchronizeImportedTextureAfterUse());
        }

        mIsOpen = false;
        mNeedsSubmit = false;
        mSharedTextures.clear();
        mHeapsPendingUsage.clear();
        mTempBuffers.clear();
    }
    return {};
}

void CommandRecordingContext::TrackHeapUsage(Heap* heap, ExecutionSerial serial) {
    // Before tracking the heap, check the last serial it was recorded on to ensure we aren't
    // tracking it more than once.
    if (heap->GetLastUsage() < serial) {
        heap->SetLastUsage(serial);
        mHeapsPendingUsage.push_back(heap);
    }
}

ID3D12GraphicsCommandList* CommandRecordingContext::GetCommandList() const {
    ASSERT(mD3d12CommandList != nullptr);
    ASSERT(IsOpen());
    return mD3d12CommandList.Get();
}

// This function will fail on Windows versions prior to 1809. Support must be queried through
// the device before calling.
ID3D12GraphicsCommandList4* CommandRecordingContext::GetCommandList4() const {
    ASSERT(IsOpen());
    ASSERT(mD3d12CommandList != nullptr);
    return mD3d12CommandList4.Get();
}

void CommandRecordingContext::Release() {
    mD3d12CommandList.Reset();
    mD3d12CommandList4.Reset();
    mIsOpen = false;
    mNeedsSubmit = false;
    mSharedTextures.clear();
    mHeapsPendingUsage.clear();
    mTempBuffers.clear();
}

bool CommandRecordingContext::IsOpen() const {
    return mIsOpen;
}

bool CommandRecordingContext::NeedsSubmit() const {
    return mNeedsSubmit;
}

void CommandRecordingContext::SetNeedsSubmit() {
    mNeedsSubmit = true;
}

void CommandRecordingContext::AddToTempBuffers(Ref<Buffer> tempBuffer) {
    mTempBuffers.emplace_back(tempBuffer);
}

}  // namespace dawn::native::d3d12
