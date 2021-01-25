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

#include "common/Assert.h"
#include "dawn_wire/client/Client.h"
#include "dawn_wire/client/Device.h"

#include <limits>

namespace dawn_wire { namespace client {

    bool Client::DoDeviceUncapturedErrorCallback(Device* device,
                                                 WGPUErrorType errorType,
                                                 const char* message) {
        switch (errorType) {
            case WGPUErrorType_NoError:
            case WGPUErrorType_Validation:
            case WGPUErrorType_OutOfMemory:
            case WGPUErrorType_Unknown:
            case WGPUErrorType_DeviceLost:
                break;
            default:
                return false;
        }
        device->HandleError(errorType, message);
        return true;
    }

    bool Client::DoDeviceLostCallback(Device* device, char const* message) {
        device->HandleDeviceLost(message);
        return true;
    }

    bool Client::DoDevicePopErrorScopeCallback(Device* device,
                                               uint64_t requestSerial,
                                               WGPUErrorType errorType,
                                               const char* message) {
        return device->OnPopErrorScopeCallback(requestSerial, errorType, message);
    }

    bool Client::DoBufferMapAsyncCallback(Buffer* buffer,
                                          uint32_t requestSerial,
                                          uint32_t status,
                                          uint64_t readInitialDataInfoLength,
                                          const uint8_t* readInitialDataInfo) {
        // The buffer might have been deleted or recreated so this isn't an error.
        if (buffer == nullptr) {
            return true;
        }

        return buffer->OnMapAsyncCallback(requestSerial, status, readInitialDataInfoLength,
                                          readInitialDataInfo);
    }

    bool Client::DoFenceUpdateCompletedValue(Fence* fence, uint64_t value) {
        // The fence might have been deleted or recreated so this isn't an error.
        if (fence == nullptr) {
            return true;
        }

        fence->OnUpdateCompletedValueCallback(value);
        return true;
    }

    bool Client::DoFenceOnCompletionCallback(Fence* fence,
                                             uint64_t requestSerial,
                                             WGPUFenceCompletionStatus status) {
        // The fence might have been deleted or recreated so this isn't an error.
        if (fence == nullptr) {
            return true;
        }

        fence->OnCompletionCallback(requestSerial, status);
        return true;
    }

    bool Client::DoDeviceCreateReadyComputePipelineCallback(Device* device,
                                                            uint64_t requestSerial,
                                                            WGPUCreateReadyPipelineStatus status,
                                                            const char* message) {
        // The device might have been deleted or recreated so this isn't an error.
        if (device == nullptr) {
            return true;
        }
        return device->OnCreateReadyComputePipelineCallback(requestSerial, status, message);
    }

    bool Client::DoDeviceCreateReadyRenderPipelineCallback(Device* device,
                                                           uint64_t requestSerial,
                                                           WGPUCreateReadyPipelineStatus status,
                                                           const char* message) {
        // The device might have been deleted or recreated so this isn't an error.
        if (device == nullptr) {
            return true;
        }
        return device->OnCreateReadyRenderPipelineCallback(requestSerial, status, message);
    }

}}  // namespace dawn_wire::client
