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
#include "dawn_wire/WireCmd_autogen.h"
#include "dawn_wire/client/Client.h"

namespace dawn_wire { namespace client {

    Device::Device(Client* client, uint32_t refcount, uint32_t id)
        : ObjectBase(this, refcount, id), mClient(client) {
        this->device = this;
    }

    Device::~Device() {
        auto errorScopes = std::move(mErrorScopes);
        for (const auto& it : errorScopes) {
            it.second.callback(WGPUErrorType_Unknown, "Device destroyed", it.second.userdata);
        }
    }

    Client* Device::GetClient() {
        return mClient;
    }

    void Device::HandleError(WGPUErrorType errorType, const char* message) {
        if (mErrorCallback) {
            mErrorCallback(errorType, message, mErrorUserdata);
        }
    }

    void Device::HandleDeviceLost(const char* message) {
        if (mDeviceLostCallback) {
            mDeviceLostCallback(message, mDeviceLostUserdata);
        }
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
        cmd.self = reinterpret_cast<WGPUDevice>(this);
        cmd.filter = filter;

        Client* wireClient = GetClient();
        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer = static_cast<char*>(wireClient->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer, *wireClient);
    }

    bool Device::RequestPopErrorScope(WGPUErrorCallback callback, void* userdata) {
        if (mErrorScopeStackSize == 0) {
            return false;
        }
        mErrorScopeStackSize--;

        uint64_t serial = mErrorScopeRequestSerial++;
        ASSERT(mErrorScopes.find(serial) == mErrorScopes.end());

        mErrorScopes[serial] = {callback, userdata};

        DevicePopErrorScopeCmd cmd;
        cmd.device = reinterpret_cast<WGPUDevice>(this);
        cmd.requestSerial = serial;

        Client* wireClient = GetClient();
        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer = static_cast<char*>(wireClient->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer, *wireClient);

        return true;
    }

    bool Device::PopErrorScope(uint64_t requestSerial, WGPUErrorType type, const char* message) {
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

}}  // namespace dawn_wire::client
