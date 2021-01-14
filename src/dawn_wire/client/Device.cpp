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

#include "dawn_wire/client/Device.h"

#include "common/Assert.h"
#include "common/Log.h"
#include "dawn_wire/client/ApiObjects_autogen.h"
#include "dawn_wire/client/Client.h"
#include "dawn_wire/client/ObjectAllocator.h"

namespace dawn_wire { namespace client {

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

        mDeviceLostCallback = [](char const*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                      "intended. If you really want to ignore device lost "
                                      "and suppress this message, set the callback to null.";
            }
        };
#endif  // DAWN_ENABLE_ASSERTS
        // Get the default queue for this device.
        auto* allocation = client->QueueAllocator().New(client);
        mDefaultQueue = allocation->object.get();

        DeviceGetDefaultQueueCmd cmd;
        cmd.self = ToAPI(this);
        cmd.result = ObjectHandle{allocation->object->id, allocation->generation};

        client->SerializeCommand(cmd);
    }

    Device::~Device() {
        // Fire pending error scopes
        auto errorScopes = std::move(mErrorScopes);
        for (const auto& it : errorScopes) {
            it.second.callback(WGPUErrorType_Unknown, "Device destroyed before callback",
                               it.second.userdata);
        }

        auto createReadyPipelineRequests = std::move(mCreateReadyPipelineRequests);
        for (const auto& it : createReadyPipelineRequests) {
            if (it.second.createReadyComputePipelineCallback != nullptr) {
                it.second.createReadyComputePipelineCallback(
                    WGPUCreateReadyPipelineStatus_DeviceDestroyed, nullptr,
                    "Device destroyed before callback", it.second.userdata);
            } else {
                ASSERT(it.second.createReadyRenderPipelineCallback != nullptr);
                it.second.createReadyRenderPipelineCallback(
                    WGPUCreateReadyPipelineStatus_DeviceDestroyed, nullptr,
                    "Device destroyed before callback", it.second.userdata);
            }
        }
    }

    void Device::HandleError(WGPUErrorType errorType, const char* message) {
        if (mErrorCallback) {
            mErrorCallback(errorType, message, mErrorUserdata);
        }
    }

    void Device::HandleDeviceLost(const char* message) {
        if (mDeviceLostCallback && !mDidRunLostCallback) {
            mDidRunLostCallback = true;
            mDeviceLostCallback(message, mDeviceLostUserdata);
        }
    }

    void Device::CancelCallbacksForDisconnect() {
        for (auto& it : mCreateReadyPipelineRequests) {
            ASSERT((it.second.createReadyComputePipelineCallback != nullptr) ^
                   (it.second.createReadyRenderPipelineCallback != nullptr));
            if (it.second.createReadyRenderPipelineCallback) {
                it.second.createReadyRenderPipelineCallback(
                    WGPUCreateReadyPipelineStatus_DeviceLost, nullptr, "Device lost",
                    it.second.userdata);
            } else {
                it.second.createReadyComputePipelineCallback(
                    WGPUCreateReadyPipelineStatus_DeviceLost, nullptr, "Device lost",
                    it.second.userdata);
            }
        }
        mCreateReadyPipelineRequests.clear();

        for (auto& it : mErrorScopes) {
            it.second.callback(WGPUErrorType_DeviceLost, "Device lost", it.second.userdata);
        }
        mErrorScopes.clear();
    }

    std::weak_ptr<bool> Device::GetAliveWeakPtr() {
        return mIsAlive;
    }

    void Device::SetUncapturedErrorCallback(WGPUErrorCallback errorCallback, void* errorUserdata) {
        mErrorCallback = errorCallback;
        mErrorUserdata = errorUserdata;
    }

    void Device::SetDeviceLostCallback(WGPUDeviceLostCallback callback, void* userdata) {
        mDeviceLostCallback = callback;
        mDeviceLostUserdata = userdata;
    }

    void Device::PushErrorScope(WGPUErrorFilter filter) {
        mErrorScopeStackSize++;

        DevicePushErrorScopeCmd cmd;
        cmd.self = ToAPI(this);
        cmd.filter = filter;

        client->SerializeCommand(cmd);
    }

    bool Device::PopErrorScope(WGPUErrorCallback callback, void* userdata) {
        if (mErrorScopeStackSize == 0) {
            return false;
        }
        mErrorScopeStackSize--;

        if (client->IsDisconnected()) {
            callback(WGPUErrorType_DeviceLost, "GPU device disconnected", userdata);
            return true;
        }

        uint64_t serial = mErrorScopeRequestSerial++;
        ASSERT(mErrorScopes.find(serial) == mErrorScopes.end());

        mErrorScopes[serial] = {callback, userdata};

        DevicePopErrorScopeCmd cmd;
        cmd.device = ToAPI(this);
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

        auto requestIt = mErrorScopes.find(requestSerial);
        if (requestIt == mErrorScopes.end()) {
            return false;
        }

        ErrorScopeData request = std::move(requestIt->second);

        mErrorScopes.erase(requestIt);
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

    WGPUQueue Device::GetDefaultQueue() {
        mDefaultQueue->refcount++;
        return ToAPI(mDefaultQueue);
    }

    void Device::CreateReadyComputePipeline(WGPUComputePipelineDescriptor const* descriptor,
                                            WGPUCreateReadyComputePipelineCallback callback,
                                            void* userdata) {
        if (client->IsDisconnected()) {
            return callback(WGPUCreateReadyPipelineStatus_DeviceLost, nullptr,
                            "GPU device disconnected", userdata);
        }

        DeviceCreateReadyComputePipelineCmd cmd;
        cmd.deviceId = this->id;
        cmd.descriptor = descriptor;

        uint64_t serial = mCreateReadyPipelineRequestSerial++;
        ASSERT(mCreateReadyPipelineRequests.find(serial) == mCreateReadyPipelineRequests.end());
        cmd.requestSerial = serial;

        auto* allocation = client->ComputePipelineAllocator().New(client);
        CreateReadyPipelineRequest request = {};
        request.createReadyComputePipelineCallback = callback;
        request.userdata = userdata;
        request.pipelineObjectID = allocation->object->id;

        cmd.pipelineObjectHandle = ObjectHandle{allocation->object->id, allocation->generation};
        client->SerializeCommand(cmd);

        mCreateReadyPipelineRequests[serial] = std::move(request);
    }

    bool Device::OnCreateReadyComputePipelineCallback(uint64_t requestSerial,
                                                      WGPUCreateReadyPipelineStatus status,
                                                      const char* message) {
        const auto& requestIt = mCreateReadyPipelineRequests.find(requestSerial);
        if (requestIt == mCreateReadyPipelineRequests.end()) {
            return false;
        }

        CreateReadyPipelineRequest request = std::move(requestIt->second);
        mCreateReadyPipelineRequests.erase(requestIt);

        auto pipelineAllocation =
            client->ComputePipelineAllocator().GetObject(request.pipelineObjectID);

        // If the return status is a failure we should give a null pipeline to the callback and
        // free the allocation both on the client side and the server side.
        if (status != WGPUCreateReadyPipelineStatus_Success) {
            client->ComputePipelineAllocator().Free(pipelineAllocation);
            request.createReadyComputePipelineCallback(status, nullptr, message, request.userdata);
            return true;
        }

        WGPUComputePipeline pipeline = reinterpret_cast<WGPUComputePipeline>(pipelineAllocation);
        request.createReadyComputePipelineCallback(status, pipeline, message, request.userdata);

        return true;
    }

    void Device::CreateReadyRenderPipeline(WGPURenderPipelineDescriptor const* descriptor,
                                           WGPUCreateReadyRenderPipelineCallback callback,
                                           void* userdata) {
        if (client->IsDisconnected()) {
            return callback(WGPUCreateReadyPipelineStatus_DeviceLost, nullptr,
                            "GPU device disconnected", userdata);
        }
        DeviceCreateReadyRenderPipelineCmd cmd;
        cmd.deviceId = this->id;
        cmd.descriptor = descriptor;

        uint64_t serial = mCreateReadyPipelineRequestSerial++;
        ASSERT(mCreateReadyPipelineRequests.find(serial) == mCreateReadyPipelineRequests.end());
        cmd.requestSerial = serial;

        auto* allocation = client->RenderPipelineAllocator().New(client);
        CreateReadyPipelineRequest request = {};
        request.createReadyRenderPipelineCallback = callback;
        request.userdata = userdata;
        request.pipelineObjectID = allocation->object->id;

        cmd.pipelineObjectHandle = ObjectHandle(allocation->object->id, allocation->generation);
        client->SerializeCommand(cmd);

        mCreateReadyPipelineRequests[serial] = std::move(request);
    }

    bool Device::OnCreateReadyRenderPipelineCallback(uint64_t requestSerial,
                                                     WGPUCreateReadyPipelineStatus status,
                                                     const char* message) {
        const auto& requestIt = mCreateReadyPipelineRequests.find(requestSerial);
        if (requestIt == mCreateReadyPipelineRequests.end()) {
            return false;
        }

        CreateReadyPipelineRequest request = std::move(requestIt->second);
        mCreateReadyPipelineRequests.erase(requestIt);

        auto pipelineAllocation =
            client->RenderPipelineAllocator().GetObject(request.pipelineObjectID);

        // If the return status is a failure we should give a null pipeline to the callback and
        // free the allocation both on the client side and the server side.
        if (status != WGPUCreateReadyPipelineStatus_Success) {
            client->RenderPipelineAllocator().Free(pipelineAllocation);
            request.createReadyRenderPipelineCallback(status, nullptr, message, request.userdata);
            return true;
        }

        WGPURenderPipeline pipeline = reinterpret_cast<WGPURenderPipeline>(pipelineAllocation);
        request.createReadyRenderPipelineCallback(status, pipeline, message, request.userdata);

        return true;
    }

}}  // namespace dawn_wire::client
