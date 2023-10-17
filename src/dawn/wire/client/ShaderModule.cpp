// Copyright 2021 The Dawn & Tint Authors
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
