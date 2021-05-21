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
        void HandleCreateRenderPipelineAsyncCallbackResult(KnownObjects<Pipeline>* knownObjects,
                                                           WGPUCreatePipelineAsyncStatus status,
                                                           Pipeline pipeline,
                                                           CreatePipelineAsyncUserData* data) {
            // May be null if the device was destroyed. Device destruction destroys child
            // objects on the wire.
            auto* pipelineObject =
                knownObjects->Get(data->pipelineObjectID, AllocationState::Reserved);
            // Should be impossible to fail. ObjectIds can't be freed by a destroy command until
            // they move from Reserved to Allocated, or if they are destroyed here.
            ASSERT(pipelineObject != nullptr);

            if (status == WGPUCreatePipelineAsyncStatus_Success) {
                // Assign the handle and allocated status if the pipeline is created successfully.
                pipelineObject->state = AllocationState::Allocated;
                pipelineObject->handle = pipeline;

                // This should be impossible to fail. It would require a command to be sent that
                // creates a duplicate ObjectId, which would fail validation.
                bool success = TrackDeviceChild(pipelineObject->deviceInfo, objectType,
                                                data->pipelineObjectID);
                ASSERT(success);
            } else {
                // Otherwise, free the ObjectId which will make it unusable.
                knownObjects->Free(data->pipelineObjectID);
                ASSERT(pipeline == nullptr);
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

    bool Server::DoDeviceCreateComputePipelineAsync(
        ObjectId deviceId,
        uint64_t requestSerial,
        ObjectHandle pipelineObjectHandle,
        const WGPUComputePipelineDescriptor* descriptor) {
        auto* device = DeviceObjects().Get(deviceId);
        if (device == nullptr) {
            return false;
        }

        auto* resultData =
            ComputePipelineObjects().Allocate(pipelineObjectHandle.id, AllocationState::Reserved);
        if (resultData == nullptr) {
            return false;
        }

        resultData->generation = pipelineObjectHandle.generation;
        resultData->deviceInfo = device->info.get();

        auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
        userdata->device = ObjectHandle{deviceId, device->generation};
        userdata->requestSerial = requestSerial;
        userdata->pipelineObjectID = pipelineObjectHandle.id;

        mProcs.deviceCreateComputePipelineAsync(
            device->handle, descriptor,
            ForwardToServer<decltype(&Server::OnCreateComputePipelineAsyncCallback)>::Func<
                &Server::OnCreateComputePipelineAsyncCallback>(),
            userdata.release());
        return true;
    }

    void Server::OnCreateComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus status,
                                                      WGPUComputePipeline pipeline,
                                                      const char* message,
                                                      CreatePipelineAsyncUserData* data) {
        HandleCreateRenderPipelineAsyncCallbackResult<ObjectType::ComputePipeline>(
            &ComputePipelineObjects(), status, pipeline, data);

        ReturnDeviceCreateComputePipelineAsyncCallbackCmd cmd;
        cmd.device = data->device;
        cmd.status = status;
        cmd.requestSerial = data->requestSerial;
        cmd.message = message;

        SerializeCommand(cmd);
    }

    bool Server::DoDeviceCreateRenderPipelineAsync(ObjectId deviceId,
                                                   uint64_t requestSerial,
                                                   ObjectHandle pipelineObjectHandle,
                                                   const WGPURenderPipelineDescriptor* descriptor) {
        auto* device = DeviceObjects().Get(deviceId);
        if (device == nullptr) {
            return false;
        }

        auto* resultData =
            RenderPipelineObjects().Allocate(pipelineObjectHandle.id, AllocationState::Reserved);
        if (resultData == nullptr) {
            return false;
        }

        resultData->generation = pipelineObjectHandle.generation;
        resultData->deviceInfo = device->info.get();

        auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
        userdata->device = ObjectHandle{deviceId, device->generation};
        userdata->requestSerial = requestSerial;
        userdata->pipelineObjectID = pipelineObjectHandle.id;

        mProcs.deviceCreateRenderPipelineAsync(
            device->handle, descriptor,
            ForwardToServer<decltype(&Server::OnCreateRenderPipelineAsyncCallback)>::Func<
                &Server::OnCreateRenderPipelineAsyncCallback>(),
            userdata.release());
        return true;
    }

    void Server::OnCreateRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus status,
                                                     WGPURenderPipeline pipeline,
                                                     const char* message,
                                                     CreatePipelineAsyncUserData* data) {
        HandleCreateRenderPipelineAsyncCallbackResult<ObjectType::RenderPipeline>(
            &RenderPipelineObjects(), status, pipeline, data);

        ReturnDeviceCreateRenderPipelineAsyncCallbackCmd cmd;
        cmd.device = data->device;
        cmd.status = status;
        cmd.requestSerial = data->requestSerial;
        cmd.message = message;

        SerializeCommand(cmd);
    }

}}  // namespace dawn_wire::server
