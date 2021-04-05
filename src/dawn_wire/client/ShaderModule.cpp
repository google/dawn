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

#include "dawn_wire/client/ShaderModule.h"

#include "dawn_wire/client/Client.h"

namespace dawn_wire { namespace client {

    ShaderModule::~ShaderModule() {
        // Callbacks need to be fired in all cases, as they can handle freeing resources. So we call
        // them with "Unknown" status.
        for (auto& it : mCompilationInfoRequests) {
            if (it.second.callback) {
                it.second.callback(WGPUCompilationInfoRequestStatus_Unknown, nullptr,
                                   it.second.userdata);
            }
        }
        mCompilationInfoRequests.clear();
    }

    void ShaderModule::GetCompilationInfo(WGPUCompilationInfoCallback callback, void* userdata) {
        if (client->IsDisconnected()) {
            callback(WGPUCompilationInfoRequestStatus_DeviceLost, nullptr, userdata);
            return;
        }

        uint64_t serial = mCompilationInfoRequestSerial++;
        ShaderModuleGetCompilationInfoCmd cmd;
        cmd.shaderModuleId = this->id;
        cmd.requestSerial = serial;

        mCompilationInfoRequests[serial] = {callback, userdata};

        client->SerializeCommand(cmd);
    }

    bool ShaderModule::GetCompilationInfoCallback(uint64_t requestSerial,
                                                  WGPUCompilationInfoRequestStatus status,
                                                  const WGPUCompilationInfo* info) {
        auto requestIt = mCompilationInfoRequests.find(requestSerial);
        if (requestIt == mCompilationInfoRequests.end()) {
            return false;
        }

        // Remove the request data so that the callback cannot be called again.
        // ex.) inside the callback: if the shader module is deleted, all callbacks reject.
        CompilationInfoRequest request = std::move(requestIt->second);
        mCompilationInfoRequests.erase(requestIt);

        request.callback(status, info, request.userdata);
        return true;
    }

}}  // namespace dawn_wire::client
