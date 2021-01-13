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

#ifndef DAWNWIRE_CLIENT_DEVICE_H_
#define DAWNWIRE_CLIENT_DEVICE_H_

#include <dawn/webgpu.h>

#include "common/LinkedList.h"
#include "dawn_wire/WireCmd_autogen.h"
#include "dawn_wire/client/ApiObjects_autogen.h"
#include "dawn_wire/client/ObjectBase.h"

#include <map>

namespace dawn_wire { namespace client {

    class Client;
    class Queue;

    class Device final : public ObjectBase {
      public:
        Device(Client* client, uint32_t refcount, uint32_t id);
        ~Device();

        void SetUncapturedErrorCallback(WGPUErrorCallback errorCallback, void* errorUserdata);
        void SetDeviceLostCallback(WGPUDeviceLostCallback errorCallback, void* errorUserdata);
        void InjectError(WGPUErrorType type, const char* message);
        void PushErrorScope(WGPUErrorFilter filter);
        bool PopErrorScope(WGPUErrorCallback callback, void* userdata);
        WGPUBuffer CreateBuffer(const WGPUBufferDescriptor* descriptor);
        WGPUBuffer CreateErrorBuffer();
        void CreateReadyComputePipeline(WGPUComputePipelineDescriptor const* descriptor,
                                        WGPUCreateReadyComputePipelineCallback callback,
                                        void* userdata);
        void CreateReadyRenderPipeline(WGPURenderPipelineDescriptor const* descriptor,
                                       WGPUCreateReadyRenderPipelineCallback callback,
                                       void* userdata);

        void HandleError(WGPUErrorType errorType, const char* message);
        void HandleDeviceLost(const char* message);
        bool OnPopErrorScopeCallback(uint64_t requestSerial,
                                     WGPUErrorType type,
                                     const char* message);
        bool OnCreateReadyComputePipelineCallback(uint64_t requestSerial,
                                                  WGPUCreateReadyPipelineStatus status,
                                                  const char* message);
        bool OnCreateReadyRenderPipelineCallback(uint64_t requestSerial,
                                                 WGPUCreateReadyPipelineStatus status,
                                                 const char* message);

        WGPUQueue GetDefaultQueue();

        void CancelCallbacksForDisconnect() override;

      private:
        struct ErrorScopeData {
            WGPUErrorCallback callback = nullptr;
            void* userdata = nullptr;
        };
        std::map<uint64_t, ErrorScopeData> mErrorScopes;
        uint64_t mErrorScopeRequestSerial = 0;
        uint64_t mErrorScopeStackSize = 0;

        struct CreateReadyPipelineRequest {
            WGPUCreateReadyComputePipelineCallback createReadyComputePipelineCallback = nullptr;
            WGPUCreateReadyRenderPipelineCallback createReadyRenderPipelineCallback = nullptr;
            void* userdata = nullptr;
            ObjectId pipelineObjectID;
        };
        std::map<uint64_t, CreateReadyPipelineRequest> mCreateReadyPipelineRequests;
        uint64_t mCreateReadyPipelineRequestSerial = 0;

        WGPUErrorCallback mErrorCallback = nullptr;
        WGPUDeviceLostCallback mDeviceLostCallback = nullptr;
        bool mDidRunLostCallback = false;
        void* mErrorUserdata = nullptr;
        void* mDeviceLostUserdata = nullptr;

        Queue* mDefaultQueue = nullptr;
    };

}}  // namespace dawn_wire::client

#endif  // DAWNWIRE_CLIENT_DEVICE_H_
