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

#ifndef SRC_DAWN_NATIVE_D3D12_PAGEABLED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_PAGEABLED3D12_H_

#include "dawn/common/LinkedList.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {
// This class is used to represent ID3D12Pageable allocations, and also serves as a node within
// the ResidencyManager's LRU cache. This node is inserted into the LRU-cache when it is first
// allocated, and any time it is scheduled to be used by the GPU. This node is removed from the
// LRU cache when it is evicted from resident memory due to budget constraints, or when the
// pageable allocation is released.
class Pageable : public LinkNode<Pageable> {
  public:
    Pageable(ComPtr<ID3D12Pageable> d3d12Pageable, MemorySegment memorySegment, uint64_t size);
    ~Pageable();

    ID3D12Pageable* GetD3D12Pageable() const;

    // We set mLastRecordingSerial to denote the serial this pageable was last recorded to be
    // used. We must check this serial against the current serial when recording usages to
    // ensure we do not process residency for this pageable multiple times.
    ExecutionSerial GetLastUsage() const;
    void SetLastUsage(ExecutionSerial serial);

    // The residency manager must know the last serial that any portion of the pageable was
    // submitted to be used so that we can ensure this pageable stays resident in memory at
    // least until that serial has completed.
    ExecutionSerial GetLastSubmission() const;
    void SetLastSubmission(ExecutionSerial serial);

    MemorySegment GetMemorySegment() const;

    uint64_t GetSize() const;

    bool IsInResidencyLRUCache() const;

    // In some scenarios, such as async buffer mapping or descriptor heaps, we must lock
    // residency to ensure the pageable cannot be evicted. Because multiple buffers may be
    // mapped in a single heap, we must track the number of resources currently locked.
    void IncrementResidencyLock();
    void DecrementResidencyLock();
    bool IsResidencyLocked() const;

  protected:
    ComPtr<ID3D12Pageable> mD3d12Pageable;

  private:
    // mLastUsage denotes the last time this pageable was recorded for use.
    ExecutionSerial mLastUsage = ExecutionSerial(0);
    // mLastSubmission denotes the last time this pageable was submitted to the GPU. Note that
    // although this variable often contains the same value as mLastUsage, it can differ in some
    // situations. When some asynchronous APIs (like WriteBuffer) are called, mLastUsage is
    // updated upon the call, but the backend operation is deferred until the next submission
    // to the GPU. This makes mLastSubmission unique from mLastUsage, and allows us to
    // accurately identify when a pageable can be evicted.
    ExecutionSerial mLastSubmission = ExecutionSerial(0);
    MemorySegment mMemorySegment;
    uint32_t mResidencyLockRefCount = 0;
    uint64_t mSize = 0;
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_PAGEABLED3D12_H_
