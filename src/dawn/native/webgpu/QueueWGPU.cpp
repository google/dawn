// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/QueueWGPU.h"

#include <vector>

#include "dawn/native/Queue.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/CommandBufferWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<Queue>> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    return AcquireRef(new Queue(device, descriptor));
}

Queue::Queue(Device* device, const QueueDescriptor* descriptor)
    : QueueBase(device, descriptor),
      mInnerQueue(device->wgpu.deviceGetQueue(device->GetInnerHandle())) {}

Queue::~Queue() {
    if (mInnerQueue) {
        ToBackend(GetDevice())->wgpu.queueRelease(mInnerQueue);
        mInnerQueue = nullptr;
    }
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    if (commandCount == 0 || commands == nullptr) {
        return {};
    }

    auto& wgpu = ToBackend(GetDevice())->wgpu;

    std::vector<WGPUCommandBuffer> innerCommandBuffers(commandCount);
    for (uint32_t i = 0; i < commandCount; ++i) {
        innerCommandBuffers[i] = ToBackend(commands[i])->Encode();
    }

    wgpu.queueSubmit(mInnerQueue, commandCount, innerCommandBuffers.data());

    for (uint32_t i = 0; i < commandCount; ++i) {
        wgpu.commandBufferRelease(innerCommandBuffers[i]);
    }

    return {};
}

MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  const void* data,
                                  size_t size) {
    auto innerBuffer = ToBackend(buffer)->GetInnerHandle();
    ToBackend(GetDevice())
        ->wgpu.queueWriteBuffer(mInnerQueue, innerBuffer, bufferOffset, data, size);
    return {};
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    // TODO(crbug.com/413053623): finish implementing WebGPU backend.
    return GetLastSubmittedCommandSerial();
}

void Queue::ForceEventualFlushOfCommands() {
    DAWN_UNREACHABLE();
}

bool Queue::HasPendingCommands() const {
    return false;
}

MaybeError Queue::SubmitPendingCommands() {
    return {};
}

ResultOrError<bool> Queue::WaitForQueueSerial(ExecutionSerial serial, Nanoseconds timeout) {
    return DAWN_UNIMPLEMENTED_ERROR("webgpu::Queue implementation is incomplete.");
}

MaybeError Queue::WaitForIdleForDestruction() {
    return {};
}

}  // namespace dawn::native::webgpu
