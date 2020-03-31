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

#ifndef DAWNNATIVE_D3D12_HEAPD3D12_H_
#define DAWNNATIVE_D3D12_HEAPD3D12_H_

#include "common/LinkedList.h"
#include "common/Serial.h"
#include "dawn_native/ResourceHeap.h"
#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

    // This class is used to represent heap allocations, but also serves as a node within the
    // ResidencyManager's LRU cache. This node is inserted into the LRU-cache when it is first
    // allocated, and any time it is scheduled to be used by the GPU. This node is removed from the
    // LRU cache when it is evicted from resident memory due to budget constraints, or when the heap
    // is destroyed.
    class Heap : public ResourceHeapBase, public LinkNode<Heap> {
      public:
        Heap(ComPtr<ID3D12Pageable> d3d12Pageable, D3D12_HEAP_TYPE heapType, uint64_t size);
        ~Heap();

        ComPtr<ID3D12Heap> GetD3D12Heap() const;
        ComPtr<ID3D12Pageable> GetD3D12Pageable() const;
        D3D12_HEAP_TYPE GetD3D12HeapType() const;

        // We set mLastRecordingSerial to denote the serial this heap was last recorded to be used.
        // We must check this serial against the current serial when recording heap usages to ensure
        // we do not process residency for this heap multiple times.
        Serial GetLastUsage() const;
        void SetLastUsage(Serial serial);

        // The residency manager must know the last serial that any portion of the heap was
        // submitted to be used so that we can ensure this heap stays resident in memory at least
        // until that serial has completed.
        uint64_t GetLastSubmission() const;
        void SetLastSubmission(Serial serial);

        uint64_t GetSize() const;

        bool IsInResidencyLRUCache() const;

        // In some scenarios, such as async buffer mapping, we must lock residency to ensure the
        // heap cannot be evicted. Because multiple buffers may be mapped in a single heap, we must
        // track the number of resources currently locked.
        void IncrementResidencyLock();
        void DecrementResidencyLock();
        bool IsResidencyLocked() const;

      private:
        ComPtr<ID3D12Pageable> mD3d12Pageable;
        D3D12_HEAP_TYPE mD3d12HeapType;
        // mLastUsage denotes the last time this heap was recorded for use.
        Serial mLastUsage = 0;
        // mLastSubmission denotes the last time this heap was submitted to the GPU.
        Serial mLastSubmission = 0;
        uint32_t mResidencyLockRefCount = 0;
        uint64_t mSize = 0;
    };
}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_HEAPD3D12_H_