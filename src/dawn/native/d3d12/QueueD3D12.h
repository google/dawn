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

#ifndef SRC_DAWN_NATIVE_D3D12_QUEUED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_QUEUED3D12_H_

#include <memory>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SerialMap.h"
#include "dawn/native/SystemEvent.h"
#include "dawn/native/d3d/QueueD3D.h"
#include "dawn/native/d3d12/CommandRecordingContext.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;

class Queue final : public d3d::Queue {
  public:
    static ResultOrError<Ref<Queue>> Create(Device* device, const QueueDescriptor* descriptor);

    void Destroy();

    MaybeError NextSerial();
    MaybeError WaitForSerial(ExecutionSerial serial);
    ResultOrError<CommandRecordingContext*> GetPendingCommandContext(
        SubmitMode submitMode = SubmitMode::Normal);
    ID3D12Fence* GetFence() const;
    ID3D12CommandQueue* GetCommandQueue() const;
    ID3D12SharingContract* GetSharingContract() const;
    MaybeError SubmitPendingCommands();

  private:
    using d3d::Queue::Queue;
    ~Queue() override;

    MaybeError Initialize();

    MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) override;
    bool HasPendingCommands() const override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;
    void ForceEventualFlushOfCommands() override;
    MaybeError WaitForIdleForDestruction() override;

    void SetEventOnCompletion(ExecutionSerial serial, HANDLE event) override;

    // Dawn API
    void SetLabelImpl() override;

    ComPtr<ID3D12Fence> mFence;
    HANDLE mFenceEvent = nullptr;

    CommandRecordingContext mPendingCommands;
    ComPtr<ID3D12CommandQueue> mCommandQueue;
    ComPtr<ID3D12SharingContract> mD3d12SharingContract;

    std::unique_ptr<CommandAllocatorManager> mCommandAllocatorManager;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_QUEUED3D12_H_
