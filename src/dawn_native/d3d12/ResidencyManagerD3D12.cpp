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
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/Forward.h"
#include "dawn_native/d3d12/HeapD3D12.h"

#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

    ResidencyManager::ResidencyManager(Device* device)
        : mDevice(device),
          mResidencyManagementEnabled(
              device->IsToggleEnabled(Toggle::UseD3D12ResidencyManagement)) {
        UpdateVideoMemoryInfo();
    }

    // Increments number of locks on a heap to ensure the heap remains resident.
    MaybeError ResidencyManager::LockMappableHeap(Heap* heap) {
        if (!mResidencyManagementEnabled) {
            return {};
        }

        // Depending on device architecture, the heap may not need tracked.
        if (!ShouldTrackHeap(heap)) {
            return {};
        }

        // If the heap isn't already resident, make it resident.
        if (!heap->IsInResidencyLRUCache() && !heap->IsResidencyLocked()) {
            DAWN_TRY(EnsureCanMakeResident(heap->GetSize()));
            ID3D12Pageable* pageable = heap->GetD3D12Pageable().Get();
            DAWN_TRY(
                CheckHRESULT(mDevice->GetD3D12Device()->MakeResident(1, &pageable),
                             "Making a heap resident due to an underlying resource being mapped."));
        }

        // Since we can't evict the heap, it's unnecessary to track the heap in the LRU Cache.
        if (heap->IsInResidencyLRUCache()) {
            heap->RemoveFromList();
        }

        heap->IncrementResidencyLock();

        return {};
    }

    // Decrements number of locks on a heap. When the number of locks becomes zero, the heap is
    // inserted into the LRU cache and becomes eligible for eviction.
    void ResidencyManager::UnlockMappableHeap(Heap* heap) {
        if (!mResidencyManagementEnabled) {
            return;
        }

        // Depending on device architecture, the heap may not need tracked.
        if (!ShouldTrackHeap(heap)) {
            return;
        }

        ASSERT(heap->IsResidencyLocked());
        ASSERT(!heap->IsInResidencyLRUCache());
        heap->DecrementResidencyLock();

        // When all locks have been removed, the resource remains resident and becomes tracked in
        // the LRU.
        if (!heap->IsResidencyLocked()) {
            mLRUCache.Append(heap);
        }
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

    // Removes from the LRU and returns the least recently used heap when possible. Returns nullptr
    // when nothing further can be evicted.
    ResultOrError<Heap*> ResidencyManager::RemoveSingleEntryFromLRU() {
        ASSERT(!mLRUCache.empty());
        Heap* heap = mLRUCache.head()->value();
        Serial lastSubmissionSerial = heap->GetLastSubmission();

        // If the next candidate for eviction was inserted into the LRU during the current serial,
        // it is because more memory is being used in a single command list than is available.
        // In this scenario, we cannot make any more resources resident and thrashing must occur.
        if (lastSubmissionSerial == mDevice->GetPendingCommandSerial()) {
            return nullptr;
        }

        // We must ensure that any previous use of a resource has completed before the resource can
        // be evicted.
        if (lastSubmissionSerial > mDevice->GetCompletedCommandSerial()) {
            DAWN_TRY(mDevice->WaitForSerial(lastSubmissionSerial));
        }

        heap->RemoveFromList();
        return heap;
    }

    // Any time we need to make something resident in local memory, we must check that we have
    // enough free memory to make the new object resident while also staying within our budget.
    // If there isn't enough memory, we should evict until there is.
    MaybeError ResidencyManager::EnsureCanMakeResident(uint64_t sizeToMakeResident) {
        if (!mResidencyManagementEnabled) {
            return {};
        }

        UpdateVideoMemoryInfo();

        uint64_t memoryUsageAfterMakeResident = sizeToMakeResident + mVideoMemoryInfo.dawnUsage;

        // Return when we can call MakeResident and remain under budget.
        if (memoryUsageAfterMakeResident < mVideoMemoryInfo.dawnBudget) {
            return {};
        }

        std::vector<ID3D12Pageable*> resourcesToEvict;

        uint64_t sizeEvicted = 0;
        while (sizeEvicted < sizeToMakeResident) {
            Heap* heap;
            DAWN_TRY_ASSIGN(heap, RemoveSingleEntryFromLRU());

            // If no heap was returned, then nothing more can be evicted.
            if (heap == nullptr) {
                break;
            }

            sizeEvicted += heap->GetSize();
            resourcesToEvict.push_back(heap->GetD3D12Pageable().Get());
        }

        if (resourcesToEvict.size() > 0) {
            DAWN_TRY(CheckHRESULT(
                mDevice->GetD3D12Device()->Evict(resourcesToEvict.size(), resourcesToEvict.data()),
                "Evicting resident heaps to free device local memory"));
        }

        return {};
    }

    // Ensure that we are only tracking heaps that exist in DXGI_MEMORY_SEGMENT_LOCAL.
    bool ResidencyManager::ShouldTrackHeap(Heap* heap) const {
        D3D12_HEAP_PROPERTIES heapProperties =
            mDevice->GetD3D12Device()->GetCustomHeapProperties(0, heap->GetD3D12HeapType());

        if (mDevice->GetDeviceInfo().isUMA) {
            // On UMA devices, MEMORY_POOL_L0 corresponds to MEMORY_SEGMENT_LOCAL, so we must track
            // heaps in MEMORY_POOL_L0. For UMA, all heaps types exist in MEMORY_POOL_L0.
            return heapProperties.MemoryPoolPreference == D3D12_MEMORY_POOL_L0;
        }

        // On non-UMA devices, MEMORY_POOL_L1 corresponds to MEMORY_SEGMENT_LOCAL, so only track the
        // heap if it is in MEMORY_POOL_L1. For non-UMA, DEFAULT heaps exist in MEMORY_POOL_L1,
        // while READBACK and UPLOAD heaps exist in MEMORY_POOL_L0.
        return heapProperties.MemoryPoolPreference == D3D12_MEMORY_POOL_L1;
    }

    // Given a list of heaps that are pending usage, this function will estimate memory needed,
    // evict resources until enough space is available, then make resident any heaps scheduled for
    // usage.
    MaybeError ResidencyManager::EnsureHeapsAreResident(Heap** heaps, size_t heapCount) {
        if (!mResidencyManagementEnabled) {
            return {};
        }

        std::vector<ID3D12Pageable*> heapsToMakeResident;
        uint64_t sizeToMakeResident = 0;

        Serial pendingCommandSerial = mDevice->GetPendingCommandSerial();
        for (size_t i = 0; i < heapCount; i++) {
            Heap* heap = heaps[i];

            // Depending on device architecture, the heap may not need tracked.
            if (!ShouldTrackHeap(heap)) {
                continue;
            }

            // Heaps that are locked resident are not tracked in the LRU cache.
            if (heap->IsResidencyLocked()) {
                continue;
            }

            if (heap->IsInResidencyLRUCache()) {
                // If the heap is already in the LRU, we must remove it and append again below to
                // update its position in the LRU.
                heap->RemoveFromList();
            } else {
                heapsToMakeResident.push_back(heap->GetD3D12Pageable().Get());
                sizeToMakeResident += heap->GetSize();
            }

            mLRUCache.Append(heap);
            heap->SetLastSubmission(pendingCommandSerial);
        }

        if (heapsToMakeResident.size() != 0) {
            DAWN_TRY(EnsureCanMakeResident(sizeToMakeResident));

            // Note that MakeResident is a synchronous function and can add a significant
            // overhead to command recording. In the future, it may be possible to decrease this
            // overhead by using MakeResident on a secondary thread, or by instead making use of
            // the EnqueueMakeResident function (which is not available on all Windows 10
            // platforms).
            DAWN_TRY(CheckHRESULT(mDevice->GetD3D12Device()->MakeResident(
                                      heapsToMakeResident.size(), heapsToMakeResident.data()),
                                  "Making scheduled-to-be-used resources resident in "
                                  "device local memory"));
        }

        return {};
    }

    // When a new heap is allocated, the heap will be made resident upon creation. We must track
    // when this happens to avoid calling MakeResident a second time.
    void ResidencyManager::TrackResidentAllocation(Heap* heap) {
        if (!mResidencyManagementEnabled) {
            return;
        }

        // Depending on device architecture and heap type, the heap may not need tracked.
        if (!ShouldTrackHeap(heap)) {
            return;
        }

        mLRUCache.Append(heap);
    }
}}  // namespace dawn_native::d3d12