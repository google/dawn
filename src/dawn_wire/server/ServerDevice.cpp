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

    namespace {

        template <ObjectType objectType, typename Pipeline>
        void HandleCreateReadyRenderPipelineCallbackResult(KnownObjects<Pipeline>* knownObjects,
                                                           WGPUCreateReadyPipelineStatus status,
                                                           Pipeline pipeline,
                                                           const char* message,
                                                           CreateReadyPipelineUserData* data) {
            auto* pipelineObject = knownObjects->Get(data->pipelineObjectID);

            if (status == WGPUCreateReadyPipelineStatus_Success) {
                ASSERT(pipelineObject != nullptr);
                pipelineObject->handle = pipeline;
            } else if (pipelineObject != nullptr) {
                // May be null if the device was destroyed. Device destruction destroys child
                // objects on the wire.
                if (!UntrackDeviceChild(pipelineObject->deviceInfo, objectType,
                                        data->pipelineObjectID)) {
                    UNREACHABLE();
                }
                knownObjects->Free(data->pipelineObjectID);
            }
        }

    }  // anonymous namespace

    void Server::OnUncapturedError(ObjectHandle device, WGPUErrorType type, const char* message) {
        ReturnDeviceUncapturedErrorCallbackCmd cmd;
        cmd.device = device;
        cmd.type = type;
        cmd.message = message;

        SerializeCommand(cmd);
    }

    void Server::OnDeviceLost(ObjectHandle device, const char* message) {
        ReturnDeviceLostCallbackCmd cmd;
        cmd.device = device;
        cmd.message = message;

        SerializeCommand(cmd);
    }

    bool Server::DoDevicePopErrorScope(ObjectId deviceId, uint64_t requestSerial) {
        auto* device = DeviceObjects().Get(deviceId);
        if (device == nullptr) {
            return false;
        }

        auto userdata = MakeUserdata<ErrorScopeUserdata>();
        userdata->requestSerial = requestSerial;
        userdata->device = ObjectHandle{deviceId, device->generation};

        ErrorScopeUserdata* unownedUserdata = userdata.release();
        bool success = mProcs.devicePopErrorScope(
            device->handle,
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
        cmd.device = userdata->device;
        cmd.requestSerial = userdata->requestSerial;
        cmd.type = type;
        cmd.message = message;

        SerializeCommand(cmd);
    }

    bool Server::DoDeviceCreateReadyComputePipeline(
        ObjectId deviceId,
        uint64_t requestSerial,
        ObjectHandle pipelineObjectHandle,
        const WGPUComputePipelineDescriptor* descriptor) {
        auto* device = DeviceObjects().Get(deviceId);
        if (device == nullptr) {
            return false;
        }

        auto* resultData = ComputePipelineObjects().Allocate(pipelineObjectHandle.id);
        if (resultData == nullptr) {
            return false;
        }

        resultData->generation = pipelineObjectHandle.generation;
        resultData->deviceInfo = device->info.get();
        if (!TrackDeviceChild(resultData->deviceInfo, ObjectType::ComputePipeline,
                              pipelineObjectHandle.id)) {
            return false;
        }

        auto userdata = MakeUserdata<CreateReadyPipelineUserData>();
        userdata->device = ObjectHandle{deviceId, device->generation};
        userdata->requestSerial = requestSerial;
        userdata->pipelineObjectID = pipelineObjectHandle.id;

        mProcs.deviceCreateReadyComputePipeline(
            device->handle, descriptor,
            ForwardToServer<decltype(&Server::OnCreateReadyComputePipelineCallback)>::Func<
                &Server::OnCreateReadyComputePipelineCallback>(),
            userdata.release());
        return true;
    }

    void Server::OnCreateReadyComputePipelineCallback(WGPUCreateReadyPipelineStatus status,
                                                      WGPUComputePipeline pipeline,
                                                      const char* message,
                                                      CreateReadyPipelineUserData* data) {
        HandleCreateReadyRenderPipelineCallbackResult<ObjectType::ComputePipeline>(
            &ComputePipelineObjects(), status, pipeline, message, data);

        ReturnDeviceCreateReadyComputePipelineCallbackCmd cmd;
        cmd.device = data->device;
        cmd.status = status;
        cmd.requestSerial = data->requestSerial;
        cmd.message = message;

        SerializeCommand(cmd);
    }

    bool Server::DoDeviceCreateReadyRenderPipeline(ObjectId deviceId,
                                                   uint64_t requestSerial,
                                                   ObjectHandle pipelineObjectHandle,
                                                   const WGPURenderPipelineDescriptor* descriptor) {
        auto* device = DeviceObjects().Get(deviceId);
        if (device == nullptr) {
            return false;
        }

        auto* resultData = RenderPipelineObjects().Allocate(pipelineObjectHandle.id);
        if (resultData == nullptr) {
            return false;
        }

        resultData->generation = pipelineObjectHandle.generation;
        resultData->deviceInfo = device->info.get();
        if (!TrackDeviceChild(resultData->deviceInfo, ObjectType::RenderPipeline,
                              pipelineObjectHandle.id)) {
            return false;
        }

        auto userdata = MakeUserdata<CreateReadyPipelineUserData>();
        userdata->device = ObjectHandle{deviceId, device->generation};
        userdata->requestSerial = requestSerial;
        userdata->pipelineObjectID = pipelineObjectHandle.id;

        mProcs.deviceCreateReadyRenderPipeline(
            device->handle, descriptor,
            ForwardToServer<decltype(&Server::OnCreateReadyRenderPipelineCallback)>::Func<
                &Server::OnCreateReadyRenderPipelineCallback>(),
            userdata.release());
        return true;
    }

    void Server::OnCreateReadyRenderPipelineCallback(WGPUCreateReadyPipelineStatus status,
                                                     WGPURenderPipeline pipeline,
                                                     const char* message,
                                                     CreateReadyPipelineUserData* data) {
        HandleCreateReadyRenderPipelineCallbackResult<ObjectType::RenderPipeline>(
            &RenderPipelineObjects(), status, pipeline, message, data);

        ReturnDeviceCreateReadyRenderPipelineCallbackCmd cmd;
        cmd.device = data->device;
        cmd.status = status;
        cmd.requestSerial = data->requestSerial;
        cmd.message = message;

        SerializeCommand(cmd);
    }

}}  // namespace dawn_wire::server
