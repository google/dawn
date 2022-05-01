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

#include <limits>

#include "dawn/common/Assert.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"

namespace dawn::wire::client {

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
    if (device == nullptr) {
        // The device might have been deleted or recreated so this isn't an error.
        return true;
    }
    device->HandleError(errorType, message);
    return true;
}

bool Client::DoDeviceLoggingCallback(Device* device,
                                     WGPULoggingType loggingType,
                                     const char* message) {
    if (device == nullptr) {
        // The device might have been deleted or recreated so this isn't an error.
        return true;
    }
    device->HandleLogging(loggingType, message);
    return true;
}

bool Client::DoDeviceLostCallback(Device* device,
                                  WGPUDeviceLostReason reason,
                                  char const* message) {
    if (device == nullptr) {
        // The device might have been deleted or recreated so this isn't an error.
        return true;
    }
    device->HandleDeviceLost(reason, message);
    return true;
}

bool Client::DoDevicePopErrorScopeCallback(Device* device,
                                           uint64_t requestSerial,
                                           WGPUErrorType errorType,
                                           const char* message) {
    if (device == nullptr) {
        // The device might have been deleted or recreated so this isn't an error.
        return true;
    }
    return device->OnPopErrorScopeCallback(requestSerial, errorType, message);
}

bool Client::DoBufferMapAsyncCallback(Buffer* buffer,
                                      uint64_t requestSerial,
                                      uint32_t status,
                                      uint64_t readDataUpdateInfoLength,
                                      const uint8_t* readDataUpdateInfo) {
    // The buffer might have been deleted or recreated so this isn't an error.
    if (buffer == nullptr) {
        return true;
    }
    return buffer->OnMapAsyncCallback(requestSerial, status, readDataUpdateInfoLength,
                                      readDataUpdateInfo);
}

bool Client::DoQueueWorkDoneCallback(Queue* queue,
                                     uint64_t requestSerial,
                                     WGPUQueueWorkDoneStatus status) {
    // The queue might have been deleted or recreated so this isn't an error.
    if (queue == nullptr) {
        return true;
    }
    return queue->OnWorkDoneCallback(requestSerial, status);
}

bool Client::DoDeviceCreateComputePipelineAsyncCallback(Device* device,
                                                        uint64_t requestSerial,
                                                        WGPUCreatePipelineAsyncStatus status,
                                                        const char* message) {
    // The device might have been deleted or recreated so this isn't an error.
    if (device == nullptr) {
        return true;
    }
    return device->OnCreateComputePipelineAsyncCallback(requestSerial, status, message);
}

bool Client::DoDeviceCreateRenderPipelineAsyncCallback(Device* device,
                                                       uint64_t requestSerial,
                                                       WGPUCreatePipelineAsyncStatus status,
                                                       const char* message) {
    // The device might have been deleted or recreated so this isn't an error.
    if (device == nullptr) {
        return true;
    }
    return device->OnCreateRenderPipelineAsyncCallback(requestSerial, status, message);
}

bool Client::DoShaderModuleGetCompilationInfoCallback(ShaderModule* shaderModule,
                                                      uint64_t requestSerial,
                                                      WGPUCompilationInfoRequestStatus status,
                                                      const WGPUCompilationInfo* info) {
    // The shader module might have been deleted or recreated so this isn't an error.
    if (shaderModule == nullptr) {
        return true;
    }
    return shaderModule->GetCompilationInfoCallback(requestSerial, status, info);
}

}  // namespace dawn::wire::client
