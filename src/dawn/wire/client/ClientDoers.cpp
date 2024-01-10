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
        case WGPUErrorType_Internal:
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
                                      WGPUFuture future,
                                      uint32_t status,
                                      uint64_t readDataUpdateInfoLength,
                                      const uint8_t* readDataUpdateInfo) {
    // The buffer might have been deleted or recreated so this isn't an error.
    if (buffer == nullptr) {
        return true;
    }
    return buffer->OnMapAsyncCallback(future, status, readDataUpdateInfoLength, readDataUpdateInfo);
}

bool Client::DoQueueWorkDoneCallback(Queue* queue,
                                     WGPUFuture future,
                                     WGPUQueueWorkDoneStatus status) {
    // The queue might have been deleted or recreated so this isn't an error.
    if (queue == nullptr) {
        return true;
    }
    return queue->OnWorkDoneCallback(future, status);
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
