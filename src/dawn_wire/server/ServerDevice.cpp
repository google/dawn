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

#include "dawn_wire/server/Server.h"

namespace dawn_wire { namespace server {

    void Server::OnUncapturedError(WGPUErrorType type, const char* message) {
        ReturnDeviceUncapturedErrorCallbackCmd cmd;
        cmd.type = type;
        cmd.message = message;

        SerializeCommand(cmd);
    }

    void Server::OnDeviceLost(const char* message) {
        ReturnDeviceLostCallbackCmd cmd;
        cmd.message = message;

        SerializeCommand(cmd);
    }

    bool Server::DoDevicePopErrorScope(WGPUDevice cDevice, uint64_t requestSerial) {
        auto userdata = MakeUserdata<ErrorScopeUserdata>();
        userdata->requestSerial = requestSerial;

        ErrorScopeUserdata* unownedUserdata = userdata.release();
        bool success = mProcs.devicePopErrorScope(
            cDevice,
            ForwardToServer<decltype(
                &Server::OnDevicePopErrorScope)>::Func<&Server::OnDevicePopErrorScope>(),
            unownedUserdata);
        if (!success) {
            delete unownedUserdata;
        }
        return success;
    }

    void Server::OnDevicePopErrorScope(WGPUErrorType type,
                                       const char* message,
                                       ErrorScopeUserdata* userdata) {
        ReturnDevicePopErrorScopeCallbackCmd cmd;
        cmd.requestSerial = userdata->requestSerial;
        cmd.type = type;
        cmd.message = message;

        SerializeCommand(cmd);
    }

    bool Server::DoDeviceCreateReadyComputePipeline(
        WGPUDevice cDevice,
        uint64_t requestSerial,
        ObjectHandle pipelineObjectHandle,
        const WGPUComputePipelineDescriptor* descriptor) {
        auto* resultData = ComputePipelineObjects().Allocate(pipelineObjectHandle.id);
        if (resultData == nullptr) {
            return false;
        }

        resultData->generation = pipelineObjectHandle.generation;

        auto userdata = MakeUserdata<CreateReadyPipelineUserData>();
        userdata->requestSerial = requestSerial;
        userdata->pipelineObjectID = pipelineObjectHandle.id;

        mProcs.deviceCreateReadyComputePipeline(
            cDevice, descriptor,
            ForwardToServer<decltype(&Server::OnCreateReadyComputePipelineCallback)>::Func<
                &Server::OnCreateReadyComputePipelineCallback>(),
            userdata.release());
        return true;
    }

    void Server::OnCreateReadyComputePipelineCallback(WGPUCreateReadyPipelineStatus status,
                                                      WGPUComputePipeline pipeline,
                                                      const char* message,
                                                      CreateReadyPipelineUserData* data) {
        auto* computePipelineObject = ComputePipelineObjects().Get(data->pipelineObjectID);
        ASSERT(computePipelineObject != nullptr);

        switch (status) {
            case WGPUCreateReadyPipelineStatus_Success:
                computePipelineObject->handle = pipeline;
                break;

            case WGPUCreateReadyPipelineStatus_Error:
                ComputePipelineObjects().Free(data->pipelineObjectID);
                break;

            // Currently this code is unreachable because WireServer is always deleted before the
            // removal of the device. In the future this logic may be changed when we decide to
            // support sharing one pair of WireServer/WireClient to multiple devices.
            case WGPUCreateReadyPipelineStatus_DeviceLost:
            case WGPUCreateReadyPipelineStatus_DeviceDestroyed:
            case WGPUCreateReadyPipelineStatus_Unknown:
            default:
                UNREACHABLE();
        }

        ReturnDeviceCreateReadyComputePipelineCallbackCmd cmd;
        cmd.status = status;
        cmd.requestSerial = data->requestSerial;
        cmd.message = message;

        SerializeCommand(cmd);
    }

    bool Server::DoDeviceCreateReadyRenderPipeline(WGPUDevice cDevice,
                                                   uint64_t requestSerial,
                                                   ObjectHandle pipelineObjectHandle,
                                                   const WGPURenderPipelineDescriptor* descriptor) {
        auto* resultData = RenderPipelineObjects().Allocate(pipelineObjectHandle.id);
        if (resultData == nullptr) {
            return false;
        }

        resultData->generation = pipelineObjectHandle.generation;

        auto userdata = MakeUserdata<CreateReadyPipelineUserData>();
        userdata->requestSerial = requestSerial;
        userdata->pipelineObjectID = pipelineObjectHandle.id;

        mProcs.deviceCreateReadyRenderPipeline(
            cDevice, descriptor,
            ForwardToServer<decltype(&Server::OnCreateReadyRenderPipelineCallback)>::Func<
                &Server::OnCreateReadyRenderPipelineCallback>(),
            userdata.release());
        return true;
    }

    void Server::OnCreateReadyRenderPipelineCallback(WGPUCreateReadyPipelineStatus status,
                                                     WGPURenderPipeline pipeline,
                                                     const char* message,
                                                     CreateReadyPipelineUserData* data) {
        auto* renderPipelineObject = RenderPipelineObjects().Get(data->pipelineObjectID);
        ASSERT(renderPipelineObject != nullptr);

        switch (status) {
            case WGPUCreateReadyPipelineStatus_Success:
                renderPipelineObject->handle = pipeline;
                break;

            case WGPUCreateReadyPipelineStatus_Error:
                RenderPipelineObjects().Free(data->pipelineObjectID);
                break;

            // Currently this code is unreachable because WireServer is always deleted before the
            // removal of the device. In the future this logic may be changed when we decide to
            // support sharing one pair of WireServer/WireClient to multiple devices.
            case WGPUCreateReadyPipelineStatus_DeviceLost:
            case WGPUCreateReadyPipelineStatus_DeviceDestroyed:
            case WGPUCreateReadyPipelineStatus_Unknown:
            default:
                UNREACHABLE();
        }

        ReturnDeviceCreateReadyRenderPipelineCallbackCmd cmd;
        cmd.status = status;
        cmd.requestSerial = data->requestSerial;
        cmd.message = message;

        SerializeCommand(cmd);
    }

}}  // namespace dawn_wire::server
