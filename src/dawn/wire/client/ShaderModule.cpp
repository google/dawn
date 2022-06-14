// Copyright 2021 The Dawn Authors
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

#include "dawn/wire/client/ShaderModule.h"

#include "dawn/wire/client/Client.h"

namespace dawn::wire::client {

ShaderModule::~ShaderModule() {
    ClearAllCallbacks(WGPUCompilationInfoRequestStatus_Unknown);
}

void ShaderModule::GetCompilationInfo(WGPUCompilationInfoCallback callback, void* userdata) {
    Client* client = GetClient();
    if (client->IsDisconnected()) {
        callback(WGPUCompilationInfoRequestStatus_DeviceLost, nullptr, userdata);
        return;
    }

    uint64_t serial = mCompilationInfoRequests.Add({callback, userdata});

    ShaderModuleGetCompilationInfoCmd cmd;
    cmd.shaderModuleId = GetWireId();
    cmd.requestSerial = serial;

    client->SerializeCommand(cmd);
}

bool ShaderModule::GetCompilationInfoCallback(uint64_t requestSerial,
                                              WGPUCompilationInfoRequestStatus status,
                                              const WGPUCompilationInfo* info) {
    CompilationInfoRequest request;
    if (!mCompilationInfoRequests.Acquire(requestSerial, &request)) {
        return false;
    }

    request.callback(status, info, request.userdata);
    return true;
}

void ShaderModule::CancelCallbacksForDisconnect() {
    ClearAllCallbacks(WGPUCompilationInfoRequestStatus_DeviceLost);
}

void ShaderModule::ClearAllCallbacks(WGPUCompilationInfoRequestStatus status) {
    mCompilationInfoRequests.CloseAll([status](CompilationInfoRequest* request) {
        if (request->callback != nullptr) {
            request->callback(status, nullptr, request->userdata);
        }
    });
}

}  // namespace dawn::wire::client
