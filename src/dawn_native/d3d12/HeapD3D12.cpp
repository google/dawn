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

#include "dawn_native/d3d12/HeapD3D12.h"

namespace dawn_native { namespace d3d12 {
    Heap::Heap(ComPtr<ID3D12Pageable> d3d12Pageable, D3D12_HEAP_TYPE d3d12HeapType, uint64_t size)
        : mD3d12Pageable(std::move(d3d12Pageable)), mD3d12HeapType(d3d12HeapType), mSize(size) {
    }

    Heap::~Heap() {
        // When a heap is destroyed, it no longer resides in resident memory, so we must evict it
        // from the LRU cache. If this heap is not manually removed from the LRU-cache, the
        // ResidencyManager will attempt to use it after it has been deallocated.
        if (IsInResidencyLRUCache()) {
            RemoveFromList();
        }
    }

    // This function should only be used when mD3D12Pageable was initialized from a ID3D12Pageable
    // that was initially created as an ID3D12Heap (i.e. SubAllocation). If the ID3D12Pageable was
    // initially created as an ID3D12Resource (i.e. DirectAllocation), then use GetD3D12Pageable().
    ComPtr<ID3D12Heap> Heap::GetD3D12Heap() const {
        ComPtr<ID3D12Heap> heap;
        HRESULT result = mD3d12Pageable.As(&heap);
        ASSERT(SUCCEEDED(result));
        return heap;
    }

    ComPtr<ID3D12Pageable> Heap::GetD3D12Pageable() const {
        return mD3d12Pageable;
    }

    D3D12_HEAP_TYPE Heap::GetD3D12HeapType() const {
        return mD3d12HeapType;
    }

    Serial Heap::GetLastUsage() const {
        return mLastUsage;
    }

    void Heap::SetLastUsage(Serial serial) {
        mLastUsage = serial;
    }

    uint64_t Heap::GetLastSubmission() const {
        return mLastSubmission;
    }

    void Heap::SetLastSubmission(Serial serial) {
        mLastSubmission = serial;
    }

    uint64_t Heap::GetSize() const {
        return mSize;
    }

    bool Heap::IsInResidencyLRUCache() const {
        return IsInList();
    }

    void Heap::IncrementResidencyLock() {
        ASSERT(mD3d12HeapType != D3D12_HEAP_TYPE_DEFAULT);
        mResidencyLockRefCount++;
    }

    void Heap::DecrementResidencyLock() {
        ASSERT(mD3d12HeapType != D3D12_HEAP_TYPE_DEFAULT);
        mResidencyLockRefCount--;
    }

    bool Heap::IsResidencyLocked() const {
        if (mResidencyLockRefCount == 0) {
            return false;
        }

        return true;
    }

}}  // namespace dawn_native::d3d12