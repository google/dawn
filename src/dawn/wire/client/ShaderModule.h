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

#ifndef SRC_DAWN_WIRE_CLIENT_SHADERMODULE_H_
#define SRC_DAWN_WIRE_CLIENT_SHADERMODULE_H_

#include "dawn/webgpu.h"

#include "dawn/wire/client/ObjectBase.h"
#include "dawn/wire/client/RequestTracker.h"

namespace dawn::wire::client {

class ShaderModule final : public ObjectBase {
  public:
    using ObjectBase::ObjectBase;
    ~ShaderModule() override;

    void GetCompilationInfo(WGPUCompilationInfoCallback callback, void* userdata);
    bool GetCompilationInfoCallback(uint64_t requestSerial,
                                    WGPUCompilationInfoRequestStatus status,
                                    const WGPUCompilationInfo* info);

  private:
    void CancelCallbacksForDisconnect() override;
    void ClearAllCallbacks(WGPUCompilationInfoRequestStatus status);

    struct CompilationInfoRequest {
        WGPUCompilationInfoCallback callback = nullptr;
        void* userdata = nullptr;
    };
    RequestTracker<CompilationInfoRequest> mCompilationInfoRequests;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_SHADERMODULE_H_
