// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_D3D12_COMMANDALLOCATORMANAGER_H_
#define SRC_DAWN_NATIVE_D3D12_COMMANDALLOCATORMANAGER_H_

#include <bitset>

#include "dawn/native/d3d12/d3d12_platform.h"

#include "dawn/common/SerialQueue.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"

namespace dawn::native::d3d12 {

class Device;

class CommandAllocatorManager {
  public:
    explicit CommandAllocatorManager(Device* device);

    // A CommandAllocator that is reserved must be used on the next ExecuteCommandLists
    // otherwise its commands may be reset before execution has completed on the GPU
    ResultOrError<ID3D12CommandAllocator*> ReserveCommandAllocator();
    MaybeError Tick(ExecutionSerial lastCompletedSerial);

  private:
    Device* device;

    // This must be at least 2 because the Device and Queue use separate command allocators
    static constexpr unsigned int kMaxCommandAllocators = 32;
    unsigned int mAllocatorCount;

    struct IndexedCommandAllocator {
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        unsigned int index;
    };

    ComPtr<ID3D12CommandAllocator> mCommandAllocators[kMaxCommandAllocators];
    std::bitset<kMaxCommandAllocators> mFreeAllocators;
    SerialQueue<ExecutionSerial, IndexedCommandAllocator> mInFlightCommandAllocators;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_COMMANDALLOCATORMANAGER_H_
