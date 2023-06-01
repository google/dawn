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

#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

namespace {

template <ObjectType objectType, typename Pipeline>
void HandleCreatePipelineAsyncCallback(KnownObjects<Pipeline>* knownObjects,
                                       WGPUCreatePipelineAsyncStatus status,
                                       Pipeline pipeline,
                                       CreatePipelineAsyncUserData* data) {
    if (status == WGPUCreatePipelineAsyncStatus_Success) {
        knownObjects->FillReservation(data->pipelineObjectID, pipeline);
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

void Server::OnDeviceLost(ObjectHandle device, WGPUDeviceLostReason reason, const char* message) {
    ReturnDeviceLostCallbackCmd cmd;
    cmd.device = device;
    cmd.reason = reason;
    cmd.message = message;

    SerializeCommand(cmd);
}

void Server::OnLogging(ObjectHandle device, WGPULoggingType type, const char* message) {
    ReturnDeviceLoggingCallbackCmd cmd;
    cmd.device = device;
    cmd.type = type;
    cmd.message = message;

    SerializeCommand(cmd);
}

WireResult Server::DoDevicePopErrorScope(Known<WGPUDevice> device, uint64_t requestSerial) {
    auto userdata = MakeUserdata<ErrorScopeUserdata>();
    userdata->requestSerial = requestSerial;
    userdata->device = device.AsHandle();

    mProcs.devicePopErrorScope(device->handle, ForwardToServer<&Server::OnDevicePopErrorScope>,
                               userdata.release());
    return WireResult::Success;
}

void Server::OnDevicePopErrorScope(ErrorScopeUserdata* userdata,
                                   WGPUErrorType type,
                                   const char* message) {
    ReturnDevicePopErrorScopeCallbackCmd cmd;
    cmd.device = userdata->device;
    cmd.requestSerial = userdata->requestSerial;
    cmd.type = type;
    cmd.message = message;

    SerializeCommand(cmd);
}

WireResult Server::DoDeviceCreateComputePipelineAsync(
    Known<WGPUDevice> device,
    uint64_t requestSerial,
    ObjectHandle pipelineObjectHandle,
    const WGPUComputePipelineDescriptor* descriptor) {
    Known<WGPUComputePipeline> pipeline;
    WIRE_TRY(ComputePipelineObjects().Allocate(&pipeline, pipelineObjectHandle,
                                               AllocationState::Reserved));

    auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
    userdata->device = device.AsHandle();
    userdata->requestSerial = requestSerial;
    userdata->pipelineObjectID = pipeline.id;

    mProcs.deviceCreateComputePipelineAsync(
        device->handle, descriptor, ForwardToServer<&Server::OnCreateComputePipelineAsyncCallback>,
        userdata.release());
    return WireResult::Success;
}

void Server::OnCreateComputePipelineAsyncCallback(CreatePipelineAsyncUserData* data,
                                                  WGPUCreatePipelineAsyncStatus status,
                                                  WGPUComputePipeline pipeline,
                                                  const char* message) {
    HandleCreatePipelineAsyncCallback<ObjectType::ComputePipeline>(&ComputePipelineObjects(),
                                                                   status, pipeline, data);

    ReturnDeviceCreateComputePipelineAsyncCallbackCmd cmd;
    cmd.device = data->device;
    cmd.status = status;
    cmd.requestSerial = data->requestSerial;
    cmd.message = message;

    SerializeCommand(cmd);
}

WireResult Server::DoDeviceCreateRenderPipelineAsync(
    Known<WGPUDevice> device,
    uint64_t requestSerial,
    ObjectHandle pipelineObjectHandle,
    const WGPURenderPipelineDescriptor* descriptor) {
    Known<WGPURenderPipeline> pipeline;
    WIRE_TRY(RenderPipelineObjects().Allocate(&pipeline, pipelineObjectHandle,
                                              AllocationState::Reserved));

    auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
    userdata->device = device.AsHandle();
    userdata->requestSerial = requestSerial;
    userdata->pipelineObjectID = pipeline.id;

    mProcs.deviceCreateRenderPipelineAsync(
        device->handle, descriptor, ForwardToServer<&Server::OnCreateRenderPipelineAsyncCallback>,
        userdata.release());
    return WireResult::Success;
}

void Server::OnCreateRenderPipelineAsyncCallback(CreatePipelineAsyncUserData* data,
                                                 WGPUCreatePipelineAsyncStatus status,
                                                 WGPURenderPipeline pipeline,
                                                 const char* message) {
    HandleCreatePipelineAsyncCallback<ObjectType::RenderPipeline>(&RenderPipelineObjects(), status,
                                                                  pipeline, data);

    ReturnDeviceCreateRenderPipelineAsyncCallbackCmd cmd;
    cmd.device = data->device;
    cmd.status = status;
    cmd.requestSerial = data->requestSerial;
    cmd.message = message;

    SerializeCommand(cmd);
}

}  // namespace dawn::wire::server
