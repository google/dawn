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

#include <memory>

#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

bool Server::DoShaderModuleGetCompilationInfo(ObjectId shaderModuleId, uint64_t requestSerial) {
    auto* shaderModule = ShaderModuleObjects().Get(shaderModuleId);
    if (shaderModule == nullptr) {
        return false;
    }

    auto userdata = MakeUserdata<ShaderModuleGetCompilationInfoUserdata>();
    userdata->shaderModule = ObjectHandle{shaderModuleId, shaderModule->generation};
    userdata->requestSerial = requestSerial;

    mProcs.shaderModuleGetCompilationInfo(
        shaderModule->handle, ForwardToServer<&Server::OnShaderModuleGetCompilationInfo>,
        userdata.release());
    return true;
}

void Server::OnShaderModuleGetCompilationInfo(ShaderModuleGetCompilationInfoUserdata* data,
                                              WGPUCompilationInfoRequestStatus status,
                                              const WGPUCompilationInfo* info) {
    ReturnShaderModuleGetCompilationInfoCallbackCmd cmd;
    cmd.shaderModule = data->shaderModule;
    cmd.requestSerial = data->requestSerial;
    cmd.status = status;
    cmd.info = info;

    SerializeCommand(cmd);
}

}  // namespace dawn::wire::server
