// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/wire/client/Device.h"

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/wire/client/ApiObjects_autogen.h"
#include "dawn/wire/client/Client.h"

namespace dawn::wire::client {

Device::Device(const ObjectBaseParams& params, const WGPUDeviceDescriptor* descriptor)
    : ObjectBase(params), mIsAlive(std::make_shared<bool>()) {
    if (descriptor && descriptor->deviceLostCallback) {
        mDeviceLostCallback = descriptor->deviceLostCallback;
        mDeviceLostUserdata = descriptor->deviceLostUserdata;
    }

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

    if (!mDeviceLostCallback) {
        mDeviceLostCallback = [](WGPUDeviceLostReason, char const*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                      "intended. If you really want to ignore device lost "
                                      "and suppress this message, set the callback to null.";
            }
        };
    }
#endif  // DAWN_ENABLE_ASSERTS
}

Device::~Device() {
    mErrorScopes.CloseAll([](ErrorScopeData* request) {
        request->callback(WGPUErrorType_Unknown, "Device destroyed before callback",
                          request->userdata);
    });

    mCreatePipelineAsyncRequests.CloseAll([this](CreatePipelineAsyncRequest* request) {
        if (request->createComputePipelineAsyncCallback != nullptr) {
            request->createComputePipelineAsyncCallback(
                WGPUCreatePipelineAsyncStatus_Success,
                ToAPI(GetClient()->Get<ComputePipeline>(request->pipelineObjectID)), "",
                request->userdata);
        } else {
            DAWN_ASSERT(request->createRenderPipelineAsyncCallback != nullptr);
            request->createRenderPipelineAsyncCallback(
                WGPUCreatePipelineAsyncStatus_Success,
                ToAPI(GetClient()->Get<RenderPipeline>(request->pipelineObjectID)), "",
                request->userdata);
        }
    });

    if (mQueue != nullptr) {
        GetProcs().queueRelease(ToAPI(mQueue));
    }
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

    mCreatePipelineAsyncRequests.CloseAll([this](CreatePipelineAsyncRequest* request) {
        if (request->createComputePipelineAsyncCallback != nullptr) {
            request->createComputePipelineAsyncCallback(
                WGPUCreatePipelineAsyncStatus_Success,
                ToAPI(GetClient()->Get<ComputePipeline>(request->pipelineObjectID)), "",
                request->userdata);
        } else {
            DAWN_ASSERT(request->createRenderPipelineAsyncCallback != nullptr);
            request->createRenderPipelineAsyncCallback(
                WGPUCreatePipelineAsyncStatus_Success,
                ToAPI(GetClient()->Get<RenderPipeline>(request->pipelineObjectID)), "",
                request->userdata);
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

void Device::PopErrorScope(WGPUErrorCallback callback, void* userdata) {
    Client* client = GetClient();
    if (client->IsDisconnected()) {
        callback(WGPUErrorType_DeviceLost, "GPU device disconnected", userdata);
        return;
    }

    uint64_t serial = mErrorScopes.Add({callback, userdata});
    DevicePopErrorScopeCmd cmd;
    cmd.deviceId = GetWireId();
    cmd.requestSerial = serial;
    client->SerializeCommand(cmd);
}

bool Device::OnPopErrorScopeCallback(uint64_t requestSerial,
                                     WGPUErrorType type,
                                     const char* message) {
    switch (type) {
        case WGPUErrorType_NoError:
        case WGPUErrorType_Validation:
        case WGPUErrorType_OutOfMemory:
        case WGPUErrorType_Internal:
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
    GetClient()->SerializeCommand(cmd);
}

WGPUBuffer Device::CreateBuffer(const WGPUBufferDescriptor* descriptor) {
    return Buffer::Create(this, descriptor);
}

WGPUQueue Device::GetQueue() {
    // The queue is lazily created because if a Device is created by
    // Reserve/Inject, we cannot send the GetQueue message until
    // it has been injected on the Server. It cannot happen immediately
    // on construction.
    if (mQueue == nullptr) {
        // Get the primary queue for this device.
        Client* client = GetClient();
        mQueue = client->Make<Queue>();

        DeviceGetQueueCmd cmd;
        cmd.self = ToAPI(this);
        cmd.result = mQueue->GetWireHandle();

        client->SerializeCommand(cmd);
    }

    mQueue->Reference();
    return ToAPI(mQueue);
}

void Device::CreateComputePipelineAsync(WGPUComputePipelineDescriptor const* descriptor,
                                        WGPUCreateComputePipelineAsyncCallback callback,
                                        void* userdata) {
    Client* client = GetClient();
    ComputePipeline* pipeline = client->Make<ComputePipeline>();

    if (client->IsDisconnected()) {
        return callback(WGPUCreatePipelineAsyncStatus_Success, ToAPI(pipeline), "", userdata);
    }

    CreatePipelineAsyncRequest request = {};
    request.createComputePipelineAsyncCallback = callback;
    request.userdata = userdata;
    request.pipelineObjectID = pipeline->GetWireId();

    uint64_t serial = mCreatePipelineAsyncRequests.Add(std::move(request));

    DeviceCreateComputePipelineAsyncCmd cmd;
    cmd.deviceId = GetWireId();
    cmd.descriptor = descriptor;
    cmd.requestSerial = serial;
    cmd.pipelineObjectHandle = pipeline->GetWireHandle();

    client->SerializeCommand(cmd);
}

bool Device::OnCreateComputePipelineAsyncCallback(uint64_t requestSerial,
                                                  WGPUCreatePipelineAsyncStatus status,
                                                  const char* message) {
    CreatePipelineAsyncRequest request;
    if (!mCreatePipelineAsyncRequests.Acquire(requestSerial, &request)) {
        return false;
    }

    Client* client = GetClient();
    ComputePipeline* pipeline = client->Get<ComputePipeline>(request.pipelineObjectID);

    // If the return status is a failure we should give a null pipeline to the callback and
    // free the allocation.
    if (status != WGPUCreatePipelineAsyncStatus_Success) {
        client->Free(pipeline);
        request.createComputePipelineAsyncCallback(status, nullptr, message, request.userdata);
        return true;
    }

    request.createComputePipelineAsyncCallback(status, ToAPI(pipeline), message, request.userdata);
    return true;
}

void Device::CreateRenderPipelineAsync(WGPURenderPipelineDescriptor const* descriptor,
                                       WGPUCreateRenderPipelineAsyncCallback callback,
                                       void* userdata) {
    Client* client = GetClient();
    RenderPipeline* pipeline = client->Make<RenderPipeline>();

    if (client->IsDisconnected()) {
        return callback(WGPUCreatePipelineAsyncStatus_Success, ToAPI(pipeline), "", userdata);
    }

    CreatePipelineAsyncRequest request = {};
    request.createRenderPipelineAsyncCallback = callback;
    request.userdata = userdata;
    request.pipelineObjectID = pipeline->GetWireId();

    uint64_t serial = mCreatePipelineAsyncRequests.Add(std::move(request));

    DeviceCreateRenderPipelineAsyncCmd cmd;
    cmd.deviceId = GetWireId();
    cmd.descriptor = descriptor;
    cmd.requestSerial = serial;
    cmd.pipelineObjectHandle = pipeline->GetWireHandle();

    client->SerializeCommand(cmd);
}

bool Device::OnCreateRenderPipelineAsyncCallback(uint64_t requestSerial,
                                                 WGPUCreatePipelineAsyncStatus status,
                                                 const char* message) {
    CreatePipelineAsyncRequest request;
    if (!mCreatePipelineAsyncRequests.Acquire(requestSerial, &request)) {
        return false;
    }

    Client* client = GetClient();
    RenderPipeline* pipeline = client->Get<RenderPipeline>(request.pipelineObjectID);

    // If the return status is a failure we should give a null pipeline to the callback and
    // free the allocation.
    if (status != WGPUCreatePipelineAsyncStatus_Success) {
        client->Free(pipeline);
        request.createRenderPipelineAsyncCallback(status, nullptr, message, request.userdata);
        return true;
    }

    request.createRenderPipelineAsyncCallback(status, ToAPI(pipeline), message, request.userdata);
    return true;
}

}  // namespace dawn::wire::client
