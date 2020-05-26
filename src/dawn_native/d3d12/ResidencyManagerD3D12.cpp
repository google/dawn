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

namespace dawn_native { namespace d3d12 {

    ResidencyManager::ResidencyManager(Device* device)
        : mDevice(device),
          mResidencyManagementEnabled(
              device->IsToggleEnabled(Toggle::UseD3D12ResidencyManagement)) {
        UpdateVideoMemoryInfo();
    }

    // Increments number of locks on a heap to ensure the heap remains resident.
    MaybeError ResidencyManager::LockAllocation(Pageable* pageable) {
        if (!mResidencyManagementEnabled) {
            return {};
        }

        // If the heap isn't already resident, make it resident.
        if (!pageable->IsInResidencyLRUCache() && !pageable->IsResidencyLocked()) {
            DAWN_TRY(EnsureCanMakeResident(pageable->GetSize(),
                                           GetMemorySegmentInfo(pageable->GetMemorySegment())));
            ID3D12Pageable* d3d12Pageable = pageable->GetD3D12Pageable();
            DAWN_TRY(CheckHRESULT(mDevice->GetD3D12Device()->MakeResident(1, &d3d12Pageable),
                                  "Making a scheduled-to-be-used resource resident"));
        }

        // Since we can't evict the heap, it's unnecessary to track the heap in the LRU Cache.
        if (pageable->IsInResidencyLRUCache()) {
            pageable->RemoveFromList();
        }

        pageable->IncrementResidencyLock();

        return {};
    }

    // Decrements number of locks on a heap. When the number of locks becomes zero, the heap is
    // inserted into the LRU cache and becomes eligible for eviction.
    void ResidencyManager::UnlockAllocation(Pageable* pageable) {
        if (!mResidencyManagementEnabled) {
            return;
        }

        ASSERT(pageable->IsResidencyLocked());
        ASSERT(!pageable->IsInResidencyLRUCache());
        pageable->DecrementResidencyLock();

        // If another lock still exists on the heap, nothing further should be done.
        if (pageable->IsResidencyLocked()) {
            return;
        }

        // When all locks have been removed, the resource remains resident and becomes tracked in
        // the corresponding LRU.
        TrackResidentAllocation(pageable);
    }

    // Returns the appropriate MemorySegmentInfo for a given MemorySegment.
    ResidencyManager::MemorySegmentInfo* ResidencyManager::GetMemorySegmentInfo(
        MemorySegment memorySegment) {
        switch (memorySegment) {
            case MemorySegment::Local:
                return &mVideoMemoryInfo.local;
            case MemorySegment::NonLocal:
                ASSERT(!mDevice->GetDeviceInfo().isUMA);
                return &mVideoMemoryInfo.nonLocal;
            default:
                UNREACHABLE();
        }
    }

    // Allows an application component external to Dawn to cap Dawn's residency budgets to prevent
    // competition for device memory. Returns the amount of memory reserved, which may be less
    // that the requested reservation when under pressure.
    uint64_t ResidencyManager::SetExternalMemoryReservation(MemorySegment segment,
                                                            uint64_t requestedReservationSize) {
        MemorySegmentInfo* segmentInfo = GetMemorySegmentInfo(segment);

        segmentInfo->externalRequest = requestedReservationSize;

        UpdateMemorySegmentInfo(segmentInfo);

        return segmentInfo->externalReservation;
    }

    void ResidencyManager::UpdateVideoMemoryInfo() {
        UpdateMemorySegmentInfo(&mVideoMemoryInfo.local);
        if (!mDevice->GetDeviceInfo().isUMA) {
            UpdateMemorySegmentInfo(&mVideoMemoryInfo.nonLocal);
        }
    }

    void ResidencyManager::UpdateMemorySegmentInfo(MemorySegmentInfo* segmentInfo) {
        DXGI_QUERY_VIDEO_MEMORY_INFO queryVideoMemoryInfo;

        ToBackend(mDevice->GetAdapter())
            ->GetHardwareAdapter()
            ->QueryVideoMemoryInfo(0, segmentInfo->dxgiSegment, &queryVideoMemoryInfo);

        // The video memory budget provided by QueryVideoMemoryInfo is defined by the operating
        // system, and may be lower than expected in certain scenarios. Under memory pressure, we
        // cap the external reservation to half the available budget, which prevents the external
        // component from consuming a disproportionate share of memory and ensures that Dawn can
        // continue to make forward progress. Note the choice to halve memory is arbitrarily chosen
        // and subject to future experimentation.
        segmentInfo->externalReservation =
            std::min(queryVideoMemoryInfo.Budget / 2, segmentInfo->externalRequest);

        segmentInfo->usage = queryVideoMemoryInfo.CurrentUsage - segmentInfo->externalReservation;

        // If we're restricting the budget for testing, leave the budget as is.
        if (mRestrictBudgetForTesting) {
            return;
        }

        // We cap Dawn's budget to 95% of the provided budget. Leaving some budget unused
        // decreases fluctuations in the operating-system-defined budget, which improves stability
        // for both Dawn and other applications on the system. Note the value of 95% is arbitrarily
        // chosen and subject to future experimentation.
        static constexpr float kBudgetCap = 0.95;
        segmentInfo->budget =
            (queryVideoMemoryInfo.Budget - segmentInfo->externalReservation) * kBudgetCap;
    }

    // Removes a heap from the LRU and returns the least recently used heap when possible. Returns
    // nullptr when nothing further can be evicted.
    ResultOrError<Pageable*> ResidencyManager::RemoveSingleEntryFromLRU(
        MemorySegmentInfo* memorySegment) {
        // If the LRU is empty, return nullptr to allow execution to continue. Note that fully
        // emptying the LRU is undesirable, because it can mean either 1) the LRU is not accurately
        // accounting for Dawn's GPU allocations, or 2) a component external to Dawn is using all of
        // the process budget and starving Dawn, which will cause thrash.
        if (memorySegment->lruCache.empty()) {
            return nullptr;
        }

        Pageable* pageable = memorySegment->lruCache.head()->value();

        Serial lastSubmissionSerial = pageable->GetLastSubmission();

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

        pageable->RemoveFromList();
        return pageable;
    }

    MaybeError ResidencyManager::EnsureCanAllocate(uint64_t allocationSize,
                                                   MemorySegment memorySegment) {
        if (!mResidencyManagementEnabled) {
            return {};
        }

        return EnsureCanMakeResident(allocationSize, GetMemorySegmentInfo(memorySegment));
    }

    // Any time we need to make something resident, we must check that we have enough free memory to
    // make the new object resident while also staying within budget. If there isn't enough
    // memory, we should evict until there is.
    MaybeError ResidencyManager::EnsureCanMakeResident(uint64_t sizeToMakeResident,
                                                       MemorySegmentInfo* memorySegment) {
        ASSERT(mResidencyManagementEnabled);

        UpdateMemorySegmentInfo(memorySegment);

        uint64_t memoryUsageAfterMakeResident = sizeToMakeResident + memorySegment->usage;

        // Return when we can call MakeResident and remain under budget.
        if (memoryUsageAfterMakeResident < memorySegment->budget) {
            return {};
        }

        std::vector<ID3D12Pageable*> resourcesToEvict;
        uint64_t sizeNeededToBeUnderBudget = memoryUsageAfterMakeResident - memorySegment->budget;
        uint64_t sizeEvicted = 0;
        while (sizeEvicted < sizeNeededToBeUnderBudget) {
            Pageable* pageable;
            DAWN_TRY_ASSIGN(pageable, RemoveSingleEntryFromLRU(memorySegment));

            // If no heap was returned, then nothing more can be evicted.
            if (pageable == nullptr) {
                break;
            }

            sizeEvicted += pageable->GetSize();
            resourcesToEvict.push_back(pageable->GetD3D12Pageable());
        }

        if (resourcesToEvict.size() > 0) {
            DAWN_TRY(CheckHRESULT(
                mDevice->GetD3D12Device()->Evict(resourcesToEvict.size(), resourcesToEvict.data()),
                "Evicting resident heaps to free memory"));
        }

        return {};
    }

    // Given a list of heaps that are pending usage, this function will estimate memory needed,
    // evict resources until enough space is available, then make resident any heaps scheduled for
    // usage.
    MaybeError ResidencyManager::EnsureHeapsAreResident(Heap** heaps, size_t heapCount) {
        if (!mResidencyManagementEnabled) {
            return {};
        }

        std::vector<ID3D12Pageable*> heapsToMakeResident;
        uint64_t localSizeToMakeResident = 0;
        uint64_t nonLocalSizeToMakeResident = 0;

        Serial pendingCommandSerial = mDevice->GetPendingCommandSerial();
        for (size_t i = 0; i < heapCount; i++) {
            Heap* heap = heaps[i];

            // Heaps that are locked resident are not tracked in the LRU cache.
            if (heap->IsResidencyLocked()) {
                continue;
            }

            if (heap->IsInResidencyLRUCache()) {
                // If the heap is already in the LRU, we must remove it and append again below to
                // update its position in the LRU.
                heap->RemoveFromList();
            } else {
                heapsToMakeResident.push_back(heap->GetD3D12Pageable());
                if (heap->GetMemorySegment() == MemorySegment::Local) {
                    localSizeToMakeResident += heap->GetSize();
                } else {
                    nonLocalSizeToMakeResident += heap->GetSize();
                }
            }

            // If we submit a command list to the GPU, we must ensure that heaps referenced by that
            // command list stay resident at least until that command list has finished execution.
            // Setting this serial unnecessarily can leave the LRU in a state where nothing is
            // eligible for eviction, even though some evictions may be possible.
            heap->SetLastSubmission(pendingCommandSerial);

            // Insert the heap into the appropriate LRU.
            TrackResidentAllocation(heap);
        }

        if (localSizeToMakeResident > 0) {
            DAWN_TRY(EnsureCanMakeResident(localSizeToMakeResident, &mVideoMemoryInfo.local));
        }

        if (nonLocalSizeToMakeResident > 0) {
            ASSERT(!mDevice->GetDeviceInfo().isUMA);
            DAWN_TRY(EnsureCanMakeResident(nonLocalSizeToMakeResident, &mVideoMemoryInfo.nonLocal));
        }

        if (heapsToMakeResident.size() != 0) {
            // Note that MakeResident is a synchronous function and can add a significant
            // overhead to command recording. In the future, it may be possible to decrease this
            // overhead by using MakeResident on a secondary thread, or by instead making use of
            // the EnqueueMakeResident function (which is not available on all Windows 10
            // platforms).
            // TODO(brandon1.jones@intel.com): If MakeResident fails, try evicting some more and
            // call MakeResident again.
            DAWN_TRY(CheckHRESULT(mDevice->GetD3D12Device()->MakeResident(
                                      heapsToMakeResident.size(), heapsToMakeResident.data()),
                                  "Making scheduled-to-be-used resources resident"));
        }

        return {};
    }

    // Inserts a heap at the bottom of the LRU. The passed heap must be resident or scheduled to
    // become resident within the current serial.
    void ResidencyManager::TrackResidentAllocation(Pageable* pageable) {
        if (!mResidencyManagementEnabled) {
            return;
        }

        ASSERT(pageable->IsInList() == false);
        GetMemorySegmentInfo(pageable->GetMemorySegment())->lruCache.Append(pageable);
    }

    // Places an artifical cap on Dawn's budget so we can test in a predictable manner. If used,
    // this function must be called before any resources have been created.
    void ResidencyManager::RestrictBudgetForTesting(uint64_t artificialBudgetCap) {
        ASSERT(mVideoMemoryInfo.local.lruCache.empty());
        ASSERT(mVideoMemoryInfo.nonLocal.lruCache.empty());
        ASSERT(!mRestrictBudgetForTesting);

        mRestrictBudgetForTesting = true;
        UpdateVideoMemoryInfo();

        // Dawn has a non-zero memory usage even before any resources have been created, and this
        // value can vary depending on the environment Dawn is running in. By adding this in
        // addition to the artificial budget cap, we can create a predictable and reproducible
        // budget for testing.
        mVideoMemoryInfo.local.budget = mVideoMemoryInfo.local.usage + artificialBudgetCap;
        if (!mDevice->GetDeviceInfo().isUMA) {
            mVideoMemoryInfo.nonLocal.budget =
                mVideoMemoryInfo.nonLocal.usage + artificialBudgetCap;
        }
    }

}}  // namespace dawn_native::d3d12