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

#include "dawn_wire/client/ApiObjects.h"
#include "dawn_wire/client/ApiProcs_autogen.h"
#include "dawn_wire/client/Client.h"

namespace dawn_wire { namespace client {

    void ClientHandwrittenBufferMapReadAsync(WGPUBuffer cBuffer,
                                             WGPUBufferMapReadCallback callback,
                                             void* userdata) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);
        buffer->MapReadAsync(callback, userdata);
    }

    void ClientHandwrittenBufferMapWriteAsync(WGPUBuffer cBuffer,
                                              WGPUBufferMapWriteCallback callback,
                                              void* userdata) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);
        buffer->MapWriteAsync(callback, userdata);
    }

    void ClientHandwrittenBufferSetSubData(WGPUBuffer cBuffer,
                                           uint64_t start,
                                           uint64_t count,
                                           const void* data) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);
        buffer->SetSubData(start, count, data);
    }

    void* ClientHandwrittenBufferGetMappedRange(WGPUBuffer cBuffer) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);
        return buffer->GetMappedRange();
    }

    const void* ClientHandwrittenBufferGetConstMappedRange(WGPUBuffer cBuffer) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);
        return buffer->GetConstMappedRange();
    }

    void ClientHandwrittenBufferUnmap(WGPUBuffer cBuffer) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);
        buffer->Unmap();
    }

    void ClientHandwrittenBufferDestroy(WGPUBuffer cBuffer) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);
        buffer->Destroy();
    }

    WGPUBuffer ClientHandwrittenDeviceCreateBuffer(WGPUDevice cDevice,
                                                   const WGPUBufferDescriptor* descriptor) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        return Buffer::Create(device, descriptor);
    }

    WGPUCreateBufferMappedResult ClientHandwrittenDeviceCreateBufferMapped(
        WGPUDevice cDevice,
        const WGPUBufferDescriptor* descriptor) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        return Buffer::CreateMapped(device, descriptor);
    }

    void ClientHandwrittenDevicePushErrorScope(WGPUDevice cDevice, WGPUErrorFilter filter) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        device->PushErrorScope(filter);
    }

    bool ClientHandwrittenDevicePopErrorScope(WGPUDevice cDevice,
                                              WGPUErrorCallback callback,
                                              void* userdata) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        return device->RequestPopErrorScope(callback, userdata);
    }

    uint64_t ClientHandwrittenFenceGetCompletedValue(WGPUFence cSelf) {
        auto fence = reinterpret_cast<Fence*>(cSelf);
        return fence->completedValue;
    }

    void ClientHandwrittenFenceOnCompletion(WGPUFence cFence,
                                            uint64_t value,
                                            WGPUFenceOnCompletionCallback callback,
                                            void* userdata) {
        Fence* fence = reinterpret_cast<Fence*>(cFence);
        if (value > fence->signaledValue) {
            ClientDeviceInjectError(reinterpret_cast<WGPUDevice>(fence->device),
                                    WGPUErrorType_Validation,
                                    "Value greater than fence signaled value");
            callback(WGPUFenceCompletionStatus_Error, userdata);
            return;
        }

        if (value <= fence->completedValue) {
            callback(WGPUFenceCompletionStatus_Success, userdata);
            return;
        }

        Fence::OnCompletionData request;
        request.completionCallback = callback;
        request.userdata = userdata;
        fence->requests.Enqueue(std::move(request), value);
    }

    WGPUFence ClientHandwrittenQueueCreateFence(WGPUQueue cSelf,
                                                WGPUFenceDescriptor const* descriptor) {
        Queue* queue = reinterpret_cast<Queue*>(cSelf);
        Device* device = queue->device;

        QueueCreateFenceCmd cmd;
        cmd.self = cSelf;
        auto* allocation = device->GetClient()->FenceAllocator().New(device);
        cmd.result = ObjectHandle{allocation->object->id, allocation->generation};
        cmd.descriptor = descriptor;

        device->GetClient()->SerializeCommand(cmd);

        WGPUFence cFence = reinterpret_cast<WGPUFence>(allocation->object.get());

        Fence* fence = reinterpret_cast<Fence*>(cFence);
        fence->queue = queue;

        uint64_t initialValue = descriptor != nullptr ? descriptor->initialValue : 0u;
        fence->signaledValue = initialValue;
        fence->completedValue = initialValue;
        return cFence;
    }

    void ClientHandwrittenQueueSignal(WGPUQueue cQueue, WGPUFence cFence, uint64_t signalValue) {
        Fence* fence = reinterpret_cast<Fence*>(cFence);
        Queue* queue = reinterpret_cast<Queue*>(cQueue);
        if (fence->queue != queue) {
            ClientDeviceInjectError(reinterpret_cast<WGPUDevice>(fence->device),
                                    WGPUErrorType_Validation,
                                    "Fence must be signaled on the queue on which it was created.");
            return;
        }
        if (signalValue <= fence->signaledValue) {
            ClientDeviceInjectError(reinterpret_cast<WGPUDevice>(fence->device),
                                    WGPUErrorType_Validation,
                                    "Fence value less than or equal to signaled value");
            return;
        }
        fence->signaledValue = signalValue;

        QueueSignalCmd cmd;
        cmd.self = cQueue;
        cmd.fence = cFence;
        cmd.signalValue = signalValue;

        queue->device->GetClient()->SerializeCommand(cmd);
    }

    void ClientHandwrittenQueueWriteBuffer(WGPUQueue cQueue,
                                           WGPUBuffer cBuffer,
                                           uint64_t bufferOffset,
                                           const void* data,
                                           size_t size) {
        Queue* queue = reinterpret_cast<Queue*>(cQueue);
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

        QueueWriteBufferInternalCmd cmd;
        cmd.queueId = queue->id;
        cmd.bufferId = buffer->id;
        cmd.bufferOffset = bufferOffset;
        cmd.data = static_cast<const uint8_t*>(data);
        cmd.size = size;

        queue->device->GetClient()->SerializeCommand(cmd);
    }

    void ClientDeviceReference(WGPUDevice) {
    }

    void ClientDeviceRelease(WGPUDevice) {
    }

    WGPUQueue ClientHandwrittenDeviceGetDefaultQueue(WGPUDevice cSelf) {
        Device* device = reinterpret_cast<Device*>(cSelf);
        return device->GetDefaultQueue();
    }

    void ClientHandwrittenDeviceSetUncapturedErrorCallback(WGPUDevice cSelf,
                                                           WGPUErrorCallback callback,
                                                           void* userdata) {
        Device* device = reinterpret_cast<Device*>(cSelf);
        device->SetUncapturedErrorCallback(callback, userdata);
    }
    void ClientHandwrittenDeviceSetDeviceLostCallback(WGPUDevice cSelf,
                                                      WGPUDeviceLostCallback callback,
                                                      void* userdata) {
        Device* device = reinterpret_cast<Device*>(cSelf);
        device->SetDeviceLostCallback(callback, userdata);
    }

}}  // namespace dawn_wire::client
