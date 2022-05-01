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

#include "dawn/wire/client/Device.h"

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/wire/client/ApiObjects_autogen.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/ObjectAllocator.h"

namespace dawn::wire::client {

Device::Device(Client* clientIn, uint32_t initialRefcount, uint32_t initialId)
    : ObjectBase(clientIn, initialRefcount, initialId), mIsAlive(std::make_shared<bool>()) {
#if defined(DAWN_ENABLE_ASSERTS)
    mErrorCallback = [](WGPUErrorType, char const*, void*) {
        static bool calledOnce = false;
        if (!calledOnce) {
            calledOnce = true;
            dawn::WarningLog() << "No Dawn device uncaptured error callback was set. This is "
                                  "probably not intended. If you really want to ignore errors "
                                  "and suppress this message, set the callback to null.";
        }
    };

    mDeviceLostCallback = [](WGPUDeviceLostReason, char const*, void*) {
        static bool calledOnce = false;
        if (!calledOnce) {
            calledOnce = true;
            dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                  "intended. If you really want to ignore device lost "
                                  "and suppress this message, set the callback to null.";
        }
    };
#endif  // DAWN_ENABLE_ASSERTS
}

Device::~Device() {
    mErrorScopes.CloseAll([](ErrorScopeData* request) {
        request->callback(WGPUErrorType_Unknown, "Device destroyed before callback",
                          request->userdata);
    });

    mCreatePipelineAsyncRequests.CloseAll([](CreatePipelineAsyncRequest* request) {
        if (request->createComputePipelineAsyncCallback != nullptr) {
            request->createComputePipelineAsyncCallback(
                WGPUCreatePipelineAsyncStatus_DeviceDestroyed, nullptr,
                "Device destroyed before callback", request->userdata);
        } else {
            ASSERT(request->createRenderPipelineAsyncCallback != nullptr);
            request->createRenderPipelineAsyncCallback(
                WGPUCreatePipelineAsyncStatus_DeviceDestroyed, nullptr,
                "Device destroyed before callback", request->userdata);
        }
    });
}

bool Device::GetLimits(WGPUSupportedLimits* limits) const {
    return mLimitsAndFeatures.GetLimits(limits);
}

bool Device::HasFeature(WGPUFeatureName feature) const {
    return mLimitsAndFeatures.HasFeature(feature);
}

size_t Device::EnumerateFeatures(WGPUFeatureName* features) const {
    return mLimitsAndFeatures.EnumerateFeatures(features);
}

void Device::SetLimits(const WGPUSupportedLimits* limits) {
    return mLimitsAndFeatures.SetLimits(limits);
}

void Device::SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount) {
    return mLimitsAndFeatures.SetFeatures(features, featuresCount);
}

void Device::HandleError(WGPUErrorType errorType, const char* message) {
    if (mErrorCallback) {
        mErrorCallback(errorType, message, mErrorUserdata);
    }
}

void Device::HandleLogging(WGPULoggingType loggingType, const char* message) {
    if (mLoggingCallback) {
        // Since client always run in single thread, calling the callback directly is safe.
        mLoggingCallback(loggingType, message, mLoggingUserdata);
    }
}

void Device::HandleDeviceLost(WGPUDeviceLostReason reason, const char* message) {
    if (mDeviceLostCallback && !mDidRunLostCallback) {
        mDidRunLostCallback = true;
        mDeviceLostCallback(reason, message, mDeviceLostUserdata);
    }
}

void Device::CancelCallbacksForDisconnect() {
    mErrorScopes.CloseAll([](ErrorScopeData* request) {
        request->callback(WGPUErrorType_DeviceLost, "Device lost", request->userdata);
    });

    mCreatePipelineAsyncRequests.CloseAll([](CreatePipelineAsyncRequest* request) {
        if (request->createComputePipelineAsyncCallback != nullptr) {
            request->createComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceLost,
                                                        nullptr, "Device lost", request->userdata);
        } else {
            ASSERT(request->createRenderPipelineAsyncCallback != nullptr);
            request->createRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceLost,
                                                       nullptr, "Device lost", request->userdata);
        }
    });
}

std::weak_ptr<bool> Device::GetAliveWeakPtr() {
    return mIsAlive;
}

void Device::SetUncapturedErrorCallback(WGPUErrorCallback errorCallback, void* errorUserdata) {
    mErrorCallback = errorCallback;
    mErrorUserdata = errorUserdata;
}

void Device::SetLoggingCallback(WGPULoggingCallback callback, void* userdata) {
    mLoggingCallback = callback;
    mLoggingUserdata = userdata;
}

void Device::SetDeviceLostCallback(WGPUDeviceLostCallback callback, void* userdata) {
    mDeviceLostCallback = callback;
    mDeviceLostUserdata = userdata;
}

bool Device::PopErrorScope(WGPUErrorCallback callback, void* userdata) {
    // TODO(crbug.com/dawn/1324) Replace bool return with void when users are updated.
    if (client->IsDisconnected()) {
        callback(WGPUErrorType_DeviceLost, "GPU device disconnected", userdata);
        return true;
    }

    uint64_t serial = mErrorScopes.Add({callback, userdata});
    DevicePopErrorScopeCmd cmd;
    cmd.deviceId = this->id;
    cmd.requestSerial = serial;
    client->SerializeCommand(cmd);
    return true;
}

bool Device::OnPopErrorScopeCallback(uint64_t requestSerial,
                                     WGPUErrorType type,
                                     const char* message) {
    switch (type) {
        case WGPUErrorType_NoError:
        case WGPUErrorType_Validation:
        case WGPUErrorType_OutOfMemory:
        case WGPUErrorType_Unknown:
        case WGPUErrorType_DeviceLost:
            break;
        default:
            return false;
    }

    ErrorScopeData request;
    if (!mErrorScopes.Acquire(requestSerial, &request)) {
        return false;
    }

    request.callback(type, message, request.userdata);
    return true;
}

void Device::InjectError(WGPUErrorType type, const char* message) {
    DeviceInjectErrorCmd cmd;
    cmd.self = ToAPI(this);
    cmd.type = type;
    cmd.message = message;
    client->SerializeCommand(cmd);
}

WGPUBuffer Device::CreateBuffer(const WGPUBufferDescriptor* descriptor) {
    return Buffer::Create(this, descriptor);
}

WGPUBuffer Device::CreateErrorBuffer() {
    return Buffer::CreateError(this);
}

WGPUQueue Device::GetQueue() {
    // The queue is lazily created because if a Device is created by
    // Reserve/Inject, we cannot send the GetQueue message until
    // it has been injected on the Server. It cannot happen immediately
    // on construction.
    if (mQueue == nullptr) {
        // Get the primary queue for this device.
        auto* allocation = client->QueueAllocator().New(client);
        mQueue = allocation->object.get();

        DeviceGetQueueCmd cmd;
        cmd.self = ToAPI(this);
        cmd.result = ObjectHandle{allocation->object->id, allocation->generation};

        client->SerializeCommand(cmd);
    }

    mQueue->refcount++;
    return ToAPI(mQueue);
}

void Device::CreateComputePipelineAsync(WGPUComputePipelineDescriptor const* descriptor,
                                        WGPUCreateComputePipelineAsyncCallback callback,
                                        void* userdata) {
    if (client->IsDisconnected()) {
        return callback(WGPUCreatePipelineAsyncStatus_DeviceLost, nullptr,
                        "GPU device disconnected", userdata);
    }

    auto* allocation = client->ComputePipelineAllocator().New(client);

    CreatePipelineAsyncRequest request = {};
    request.createComputePipelineAsyncCallback = callback;
    request.userdata = userdata;
    request.pipelineObjectID = allocation->object->id;

    uint64_t serial = mCreatePipelineAsyncRequests.Add(std::move(request));

    DeviceCreateComputePipelineAsyncCmd cmd;
    cmd.deviceId = this->id;
    cmd.descriptor = descriptor;
    cmd.requestSerial = serial;
    cmd.pipelineObjectHandle = ObjectHandle{allocation->object->id, allocation->generation};

    client->SerializeCommand(cmd);
}

bool Device::OnCreateComputePipelineAsyncCallback(uint64_t requestSerial,
                                                  WGPUCreatePipelineAsyncStatus status,
                                                  const char* message) {
    CreatePipelineAsyncRequest request;
    if (!mCreatePipelineAsyncRequests.Acquire(requestSerial, &request)) {
        return false;
    }

    auto pipelineAllocation =
        client->ComputePipelineAllocator().GetObject(request.pipelineObjectID);

    // If the return status is a failure we should give a null pipeline to the callback and
    // free the allocation.
    if (status != WGPUCreatePipelineAsyncStatus_Success) {
        client->ComputePipelineAllocator().Free(pipelineAllocation);
        request.createComputePipelineAsyncCallback(status, nullptr, message, request.userdata);
        return true;
    }

    WGPUComputePipeline pipeline = reinterpret_cast<WGPUComputePipeline>(pipelineAllocation);
    request.createComputePipelineAsyncCallback(status, pipeline, message, request.userdata);

    return true;
}

void Device::CreateRenderPipelineAsync(WGPURenderPipelineDescriptor const* descriptor,
                                       WGPUCreateRenderPipelineAsyncCallback callback,
                                       void* userdata) {
    if (client->IsDisconnected()) {
        return callback(WGPUCreatePipelineAsyncStatus_DeviceLost, nullptr,
                        "GPU device disconnected", userdata);
    }

    auto* allocation = client->RenderPipelineAllocator().New(client);

    CreatePipelineAsyncRequest request = {};
    request.createRenderPipelineAsyncCallback = callback;
    request.userdata = userdata;
    request.pipelineObjectID = allocation->object->id;

    uint64_t serial = mCreatePipelineAsyncRequests.Add(std::move(request));

    DeviceCreateRenderPipelineAsyncCmd cmd;
    cmd.deviceId = this->id;
    cmd.descriptor = descriptor;
    cmd.requestSerial = serial;
    cmd.pipelineObjectHandle = ObjectHandle(allocation->object->id, allocation->generation);

    client->SerializeCommand(cmd);
}

bool Device::OnCreateRenderPipelineAsyncCallback(uint64_t requestSerial,
                                                 WGPUCreatePipelineAsyncStatus status,
                                                 const char* message) {
    CreatePipelineAsyncRequest request;
    if (!mCreatePipelineAsyncRequests.Acquire(requestSerial, &request)) {
        return false;
    }

    auto pipelineAllocation = client->RenderPipelineAllocator().GetObject(request.pipelineObjectID);

    // If the return status is a failure we should give a null pipeline to the callback and
    // free the allocation.
    if (status != WGPUCreatePipelineAsyncStatus_Success) {
        client->RenderPipelineAllocator().Free(pipelineAllocation);
        request.createRenderPipelineAsyncCallback(status, nullptr, message, request.userdata);
        return true;
    }

    WGPURenderPipeline pipeline = reinterpret_cast<WGPURenderPipeline>(pipelineAllocation);
    request.createRenderPipelineAsyncCallback(status, pipeline, message, request.userdata);

    return true;
}

}  // namespace dawn::wire::client
